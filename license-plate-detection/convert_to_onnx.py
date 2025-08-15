from ultralytics import YOLO

model = YOLO('runs/detect/train6/weights/best.pt')
model.export(format='onnx', imgsz=640, opset=12, dynamic=True)  # dynamic=True allows variable batch size
