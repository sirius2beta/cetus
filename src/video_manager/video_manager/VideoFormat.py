from pathlib import Path


def getSimulationFormatCMD(video_path, width, height, framerate, encoder, IP, port, img_directory):
	"""Build a looping MP4 playback pipeline with the normal RTP outputs."""
	uri = Path(video_path).resolve().as_uri()
	source = (
		# uridecodebin selects nvv4l2decoder on Jetson.  Its output is NVMM
		# memory, which videoconvert cannot negotiate directly; nvvidconv first
		# converts it to ordinary I420 buffers.
		'uridecodebin uri="{}" ! queue ! nvvidconv ! video/x-raw,format=I420 ! '
		'videoconvert ! videoscale ! videorate ! '
		'video/x-raw,width={},height={},framerate={}/1 ! tee name=t '
	).format(uri, width, height, framerate)
	if encoder == 'h264':
		return (
			source +
			't. ! queue ! x264enc tune=zerolatency speed-preset=superfast ! '
			# Keep the MP4's timestamps: sync=false would transmit every frame as
			# fast as it can be encoded, which makes the receiver play in fast-forward.
			'rtph264pay pt=96 config-interval=1 ! udpsink host={} port={} sync=true '
			't. ! queue ! valve name=photo_valve drop=True ! videorate ! '
			'video/x-raw,framerate=1/1 ! jpegenc ! '
			'multifilesink location={}/img_%05d.jpg async=false'
		).format(IP, port, img_directory)
	return (
		source +
		't. ! queue ! jpegenc quality=30 ! rtpjpegpay ! '
		'udpsink host={} port={} sync=true '
		't. ! queue ! valve name=photo_valve drop=True ! videorate ! '
			'video/x-raw,framerate=1/1 ! jpegenc ! '
			'multifilesink location={}/img_%05d.jpg async=false'
	).format(IP, port, img_directory)


def getFormatCMD(sys, cam, format, width, height, framerate, encoder, IP, port, img_directory):
		gstring = 'v4l2src device=/dev/cetusvideo'+str(cam)
		mid = 'nan'
		if format == 'YUYV':
			format = 'YUY2'
			gstring += ' num-buffers=-1 ! video/x-raw,format={},width={},height={},framerate={}/1 ! '.format(format, width, height, framerate)
			if mid != 'nan':
				gstring += (mid+' ! ')
			if encoder == 'h264':
				if sys == 'buster':
					gstring +='videoconvert ! omxh264enc ! rtph264pay pt=96 config-interval=1 ! udpsink host={} port={}'.format(IP, port)
				else: # for jetson
					#gstring +='nvvideoconvert ! nvv4l2h264enc ! rtph264pay pt=96 config-interval=1 ! udpsink host={} port={}'.format(IP, port)	
					# Software encode, more stable?
					gstring +='videoconvert ! x264enc tune=zerolatency speed-preset=superfast ! rtph264pay pt=96 config-interval=1 ! udpsink host={} port={}'.format(IP, port)	
			
			else:
				gstring +='jpegenc quality=30 ! rtpjpegpay ! udpsink host={} port={}'.format(IP, port)
		elif format == 'MJPG':
			gstring += ' num-buffers=-1 ! image/jpeg,width={},height={},framerate={}/1 ! '.format(width, height, framerate)
			if mid != 'nan':
				gstring += (mid+' ! ')
			if encoder == 'h264':
				if sys == 'buster':
					gstring +='jpegparse ! jpegdec ! videoconvert ! omxh264enc ! rtph264pay pt=96 config-interval=1 ! udpsink host={} port={}'.format(IP, port)
				else: # for jetson
					# Jetson hardware encode H264
					#gstring +='jpegparse ! jpegdec ! videoconvert ! videoconvert ! nvvideoconvert ! nvv4l2h264enc ! rtph264pay pt=96 config-interval=1 ! udpsink host={} port={}'.format(IP, port)	
					# Software encode, more stable?
					gstring +='jpegparse ! nvv4l2decoder mjpeg=1 ! nvvidconv ! video/x-raw,format=I420 ! tee name=t t. ! queue ! x264enc bitrate=1000 tune=zerolatency speed-preset=superfast ! rtph264pay pt=96 config-interval=1 ! udpsink host={} port={} sync=false  t. ! queue ! valve name=photo_valve drop=True ! videorate ! video/x-raw,framerate=1/1 ! jpegenc ! multifilesink location={}/img_%05d.jpg async=false'.format(IP, port, img_directory)	
			else:
				gstring +='jpegparse ! jpegdec ! jpegenc quality=30 ! rtpjpegpay ! udpsink host={} port={}'.format(IP, port)

		elif format == 'GREY':
			gstring += ' num-buffers=-1 ! video/x-raw,format=GRAY8 ! videoscale ! videoconvert ! video/x-raw, format=YUY2, width=640,height=480 ! '
			if mid != 'nan':
				gstring += (mid+' ! ')
			if encoder == 'h264':
				if sys == 'buster':
					gstring +='videoconvert ! omxh264enc ! rtph264pay pt=96 config-interval=1 ! udpsink host={} port={}'.format(IP, port)
				else: # for jetson
					#gstring +='videoconvert !  nvvideoconvert ! nvv4l2h264enc ! rtph264pay pt=96 config-interval=1 ! udpsink host={} port={}'.format(IP, port)
					# Software encode, more stable?
					gstring +='videoconvert ! x264enc tune=zerolatency speed-preset=superfast ! rtph264pay pt=96 config-interval=1 ! udpsink host={} port={}'.format(IP, port)	
			
			else:
				gstring +='jpegenc quality=30 ! rtpjpegpay ! udpsink host={} port={}'.format(IP, port)
		elif format == 'H264':
			gstring += ' num-buffers=-1 ! video/x-h264 ! '
			gstring +=' rtph264pay pt=96 config-interval=1 ! udpsink host={} port={}'.format(IP, port)
		else:
			if format == 'RGBP':
				format = 'RGB16'
			elif format == 'BGR8':
				format = 'BGR'
			elif format == 'Y1':
				format = 'UYVY'
			else:
				return ''
			gstring += ' num-buffers=-1 ! video/x-raw,format={}! videoscale ! videoconvert ! videoflip method=rotate-180 ! video/x-raw, format=YUY2, width=640,height=480 ! '.format(format)
			

		return gstring
