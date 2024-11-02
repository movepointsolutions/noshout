inputs="`pacmd list-sink-inputs alsa_output.pci-0000_00_1f.3.analog-stereo`"

tlg() {
	pacmd list-sink-inputs alsa_output.pci-0000_00_1f.3.analog-stereo | awk -f sink-inputs.awk | grep Telegram | head -n1 | awk '{print $1;}'
}

mute() {
	while true; do
		c=`tlg`
		if [ -n "$c" ]; then
			echo $c
			pactl set-sink-input-volume $c 0.01
			sleep 0.3
			pactl set-sink-input-volume $c 0.07
			sleep 0.3
			pactl set-sink-input-volume $c 0.7
			sleep 0.4
			pactl set-sink-input-volume $c 1
		fi
	done
}

mute
