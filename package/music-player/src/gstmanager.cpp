#include "gstmanager.h"
#include "qdebug.h"

GstManager::GstManager(int argc, char *argv[], PathManager& _path_manager)
    : path_manager(_path_manager),
      current_id_in_playback(-1),
      player_state(GST_MNGR_STOPPED_PAUSED)
{
    gst_init(&argc, &argv);

    source = gst_element_factory_make ("uridecodebin", "source");
    if (!source)
		throw std::runtime_error("could not create source");
    convert = gst_element_factory_make ("audioconvert", "convert");
	if (!convert)
		throw std::runtime_error("could not create convert");
    resample = gst_element_factory_make ("audioresample", "resample");
	if (!resample)
		throw std::runtime_error("could not create resample");
    spectrum = gst_element_factory_make ("spectrum", "spectrum");
	if (!spectrum)
		throw std::runtime_error("could not create spectrum");
    sink = gst_element_factory_make ("autoaudiosink", "sink");
	if (!sink)
		throw std::runtime_error("could not create sink");

    pipeline = gst_pipeline_new ("test-pipeline");

    if (!pipeline || !source || !convert || !resample || !spectrum || !sink) {
        throw std::runtime_error("Not all elements could be created");
    }

    gst_bin_add_many (GST_BIN (pipeline), source, convert, resample, spectrum, sink, NULL);
    if (gst_element_link_many (convert, resample, spectrum, sink, nullptr) != TRUE) {
        gst_object_unref (pipeline);
        throw std::runtime_error("Elements could not be linked");
    }

    g_signal_connect (source, "pad-added", G_CALLBACK(pad_added_handler), this);
    bus = gst_element_get_bus (pipeline);

    gst_bus_add_watch(bus, gst_bus_callback, this);
}

GstManager::~GstManager(void)
{
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

void GstManager::pad_added_handler(GstElement *src, GstPad *pad, GstManager *this_ref)
{
    GstPad *sink_pad = gst_element_get_static_pad(this_ref->convert, "sink");
    GstPadLinkReturn ret;
    (void)src;

    if (gst_pad_is_linked (sink_pad)) {
        qErrnoWarning("Sink pad is already linked. Ignoring");
        goto exit;
    }

    ret = gst_pad_link (pad, sink_pad);
    if (GST_PAD_LINK_FAILED (ret)) {
        qErrnoWarning("Pad link failed\n");
    }

exit:
    gst_object_unref (sink_pad);
}

int GstManager::timer_cb(GstManager* this_ref)
{
    emit this_ref->progress_changed();
    return (this_ref->get_state() == GST_MNGR_PLAYING) ? true : false;
}

gboolean GstManager::gst_bus_callback(GstBus* bus, GstMessage* message, gpointer user_data)
{
    GstManager* this_ref = static_cast<GstManager*>(user_data);

    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_ERROR:{
        GError *err;
        gchar *debug;
        gst_message_parse_error(message, &err, &debug);
        qErrnoWarning("Error: %s\n", err->message);
        g_error_free (err);
        g_free (debug);
        break;
    }

    case GST_MESSAGE_ELEMENT: {
        const GstStructure *message_struct = gst_message_get_structure(message);
        const gchar *name = gst_structure_get_name(message_struct);
        const GValue *magnitudes, *mag;
        QVector<float> mag_float;

        if (strcmp (name, "spectrum") == 0) {
            magnitudes = gst_structure_get_value(message_struct, "magnitude");
            for (int i = 0; i < this_ref->spectrum_bars_count; ++i) {
                mag = gst_value_list_get_value(magnitudes, i);
                mag_float.append(g_value_get_float(mag));
            }
            if (this_ref->spectrum_graph_ptr) {
                this_ref->spectrum_graph_ptr->setData(&mag_float);
            }
        }
        break;
    }

    case GST_MESSAGE_EOS:
        this_ref->stop();
        emit this_ref->track_eos();
        break;

    default:
        // Unhandled message
        break;
    }

    return TRUE;
}

void GstManager::play(int id)
{
    GstStateChangeReturn ret;
    QString file_to_play = "file://" + path_manager.get_full_path_for_id(id);

    if ((get_state() == GST_MNGR_PLAYING) && (current_id_in_playback != id)) {
        stop();
    }

    g_object_set(source, "uri", file_to_play.toStdString().c_str(), nullptr);
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        qErrnoWarning("Unable to set the pipeline to the playing state.\n");
        return;
    }

    g_timeout_add (200, (GSourceFunc)timer_cb, this);
    player_state = GST_MNGR_PLAYING;

    emit state_changed();
}

void GstManager::pause(void)
{
    int ret = gst_element_set_state(pipeline, GST_STATE_PAUSED);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qErrnoWarning("Unable to set the pipeline to the paused state.\n");
        return;
    }
    player_state = GST_MNGR_STOPPED_PAUSED;

    emit state_changed();
}

void GstManager::stop(void)
{
    int ret = gst_element_set_state(pipeline, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qErrnoWarning("Unable to set the pipeline to the paused state.\n");
        return;
    }
    player_state = GST_MNGR_STOPPED_PAUSED;

    emit state_changed();
}

void GstManager::set_spectrum_bars_count(int i)
{
    spectrum_bars_count = i;

    g_object_set (G_OBJECT(spectrum),
                        "bands", spectrum_bars_count,
                        "threshold", -80,
                        "interval", 100 * GST_MSECOND,
                        NULL);
}

int GstManager::get_state(void)
{
    /*GstState tmp;

    gst_element_get_state(pipeline, &tmp, NULL, GST_CLOCK_TIME_NONE);

    switch (player_state) {
        case GST_STATE_PLAYING:
            return GST_MNGR_PLAYING;

        default:
            return GST_MNGR_PAUSED_IDLE;
    }*/
    return player_state;
}

int GstManager::get_progress(void)
{
    gint64 pos, len;
    int pos_sec, len_sec;
    char tmp_string[12];

    if (gst_element_query_position(pipeline, GST_FORMAT_TIME, &pos)
            && gst_element_query_duration(pipeline, GST_FORMAT_TIME, &len)) {
        pos_sec = (int)(pos/1000000000);
        len_sec = (int)(len/1000000000);
        sprintf(tmp_string, "%02d:%02d/%02d:%02d",
                                pos_sec / 60, pos_sec % 60,
                                len_sec / 60, len_sec % 60);
        playback_time_string = tmp_string;
        return (int)((pos*1000)/len);
    } else {
        playback_time_string = "--:--/--:--";
    }

    return 0;
}

void GstManager::seek(float value)
{
    gint64 new_pos, len;

    if (!gst_element_query_duration(pipeline, GST_FORMAT_TIME, &len)) {
        qErrnoWarning("cannot get current position");
        return;
    }
    new_pos = len * (gint64)value/1000;

    if (!gst_element_seek(pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                    GST_SEEK_TYPE_SET, new_pos, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
        qErrnoWarning("Seek failed!\n");
    }

    emit progress_changed();
}
