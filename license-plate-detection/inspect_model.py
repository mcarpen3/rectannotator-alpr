from ultralytics import YOLO

model = YOLO("runs/detect/train6/weights/best.pt")

print("Input size (image size): ", model.model.args['imgsz'])