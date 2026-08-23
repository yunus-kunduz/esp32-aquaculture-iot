import cv2
from ultralytics import YOLO
import requests

# Dünyanın en popüler nesne tanıma yapay zekası (Küçük ve hızlı versiyonu)
model = YOLO("yolov8n.pt") 

# ESP32-CAM'in IP adresini yazın
esp32_ip = "192.168.1.XX" 
video_url = f"http://{esp32_ip}:81/stream" # Video akış portu
cmd_url = f"http://{esp32_ip}"             # Komut portu

cap = cv2.VideoCapture(video_url)
last_state = None

while cap.isOpened():
    success, frame = cap.read()
    if not success: break

    # Yapay zeka karedeki nesneleri bulur
    results = model(frame, verbose=False)
    
    human_detected = False
    for r in results:
        for box in r.boxes:
            # 0 numaralı sınıf YOLO kütüphanesinde "person" (insan) demektir
            if int(box.cls[0]) == 0: 
                human_detected = True
                break

    # Durum değiştiyse ESP32-CAM'e Wi-Fi üzerinden emir gönder
    if human_detected != last_state:
        if human_detected:
            print("İnsan algılandı! Yeşil LED yakılıyor...")
            try: requests.get(f"{cmd_url}/HUMAN=YES", timeout=1)
            except: pass
        else:
            print("Oda boşaldı! Sarı LED yakılıyor...")
            try: requests.get(f"{cmd_url}/HUMAN=NO", timeout=1)
            except: pass
        last_state = human_detected

    # Bilgisayar ekranında yapay zekanın ne gördüğünü canlı gösterir
    annotated_frame = results[0].plot()
    cv2.imshow("YOLOv8 ESP32-CAM İzleme", annotated_frame)
    if cv2.waitKey(1) & 0xFF == ord("q"): break

cap.release()
cv2.destroyAllWindows()
