#ifndef GSTMANAGER_H
#define GSTMANAGER_H

#include <gst/gst.h>
#include <QObject>
#include "path_mngr.h"
#include "spectrumgraph.h"

class GstManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int state READ get_state NOTIFY state_changed)
    Q_PROPERTY(int progress READ get_progress NOTIFY progress_changed)
    Q_PROPERTY(QString playback_time MEMBER playback_time_string NOTIFY progress_changed)

public:
    enum GstManagerState {
        GST_MNGR_STOPPED_PAUSED = 0,
        GST_MNGR_PLAYING,
    };

    GstManager(int argc, char *argv[], PathManager& path_manager);
    ~GstManager(void);

public slots:
    void play(int id = -1);
    void pause(void);
    void stop(void);
    int get_state(void);
    int get_progress(void);
    void seek(float value);
    void set_spectrum_bars_count(int i);
    void set_spectrum_graph_ptr(QObject* tmp) { spectrum_graph_ptr = (SpectrumGraph*) tmp; }

signals:
    void state_changed();
    void progress_changed();
    void track_eos();

private:
    PathManager& path_manager;
    GstElement *pipeline, *source, *convert, *resample, *spectrum, *sink;
    GstBus *bus;
    int current_id_in_playback;
    int spectrum_bars_count;
    QString playback_time_string;
    SpectrumGraph* spectrum_graph_ptr;
    GstManagerState player_state;

    static void pad_added_handler(GstElement *src, GstPad *pad, GstManager *this_ref);
    static int timer_cb(GstManager* this_ref);
    static gboolean gst_bus_callback(GstBus* bus, GstMessage* message, gpointer user_data);
};

#endif // GSTMANAGER_H
