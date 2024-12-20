PS1='\u@\w # '

alias speaker-test-24-48000="speaker-test -D hw:0,0 -c 2 -r 48000 -F S24_LE -t sine -f 1000"
alias speaker-test-16-48000="speaker-test -D hw:0,0 -c 2 -r 48000 -F S16_LE -t sine -f 1000"
alias speaker-test-24-44100="speaker-test -D hw:0,0 -c 2 -r 44100 -F S24_LE -t sine -f 1000"
alias speaker-test-16-44100="speaker-test -D hw:0,0 -c 2 -r 44100 -F S16_LE -t sine -f 1000"

alias arecord-24-48000="arecord -D hw:0,3 -c 2 -r 48000 -f S24_LE"
alias arecord-16-48000="arecord -D hw:0,3 -c 2 -r 48000 -f S16_LE"
alias arecord-24-44100="arecord -D hw:0,3 -c 2 -r 44100 -f S24_LE"
alias arecord-16-44100="arecord -D hw:0,3 -c 2 -r 44100 -f S16_LE"