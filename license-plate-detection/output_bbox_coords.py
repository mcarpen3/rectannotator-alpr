from ultralytics import YOLO

# Load model
model = YOLO("runs/detect/train6/weights/best.pt")

# Run prediction on an image
results = model("e:/frame_0044.png")[0]  # first (and only) result

# Print all detections
for box in results.boxes:
    x1, y1, x2, y2 = box.xyxy[0].tolist()   # Bounding box (pixels)
    conf = box.conf.item()                 # Confidence score
    cls = int(box.cls.item())              # Class ID
    print(f"Class {cls} | Conf: {conf:.2f} | Box: ({x1:.1f}, {y1:.1f}, {x2:.1f}, {y2:.1f})")
