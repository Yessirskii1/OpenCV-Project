import cv2
import serial
import time

#  searial порт
ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
time.sleep(2)

# rtsp
url = "rtsp://admin:gsk7643gsk@192.168.10.99:554/Streaming/Channels/101"


def connect_camera():
    print("Connecting to camera...")

    cap = cv2.VideoCapture(url, cv2.CAP_FFMPEG)

    if not cap.isOpened():
        print("Camera connection failed")
        return None

    return cap


cap = connect_camera()

if cap is None:
    exit()

#  frame
ret, previous_frame = cap.read()

if not ret or previous_frame is None:
    print("Camera init failed")
    cap.release()
    exit()

# alarm state
alarm_sent = False
last_motion_time = 0

print("Motion detector started")

try:

    while True:

        ret, current_frame = cap.read()

        # camera lost
        if not ret or current_frame is None:

            print("Camera disconnected, reconnecting...")

            cap.release()
            time.sleep(2)

            cap = connect_camera()

            if cap is None:
                time.sleep(5)
                continue

            ret, previous_frame = cap.read()

            if not ret:
                time.sleep(2)
                continue

            continue

        #  frame size check
        if previous_frame.shape != current_frame.shape:

            print("Frame mismatch, skipping...")

            previous_frame = current_frame.copy()
            continue

        # motion detection
        diff = cv2.absdiff(previous_frame, current_frame)
 gray = cv2.cvtColor(diff, cv2.COLOR_BGR2GRAY)
        blur = cv2.GaussianBlur(gray, (5, 5), 0)

        _, thresh = cv2.threshold(
            blur,
            25,
            255,
            cv2.THRESH_BINARY
        )

        contours, _ = cv2.findContours(
            thresh,
            cv2.RETR_EXTERNAL,
            cv2.CHAIN_APPROX_SIMPLE
        )

        motion = False

        for c in contours:

            if cv2.contourArea(c) > 5000:
                motion = True
                break

        # alarm
        if motion:

            last_motion_time = time.time()

            if not alarm_sent:

                print("MOTION DETECTED 🚨")

                ser.write(b"ALARM\n")

                alarm_sent = True

        else:

            # Сброс тревоги через 10 секунд без движения
            if alarm_sent and (time.time() - last_motion_time > 10):

                print("Motion cleared")

                alarm_sent = False

        previous_frame = current_frame.copy()

except KeyboardInterrupt:

    print("\nStopping...")

finally:

    cap.release()

    try:
        ser.close()
    except:
        pass

    print("Stopped")
