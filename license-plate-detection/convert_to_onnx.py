from ultralytics import YOLO

model = YOLO('runs/detect/train8/weights/best.pt')
output_path = model.export(format='onnx', imgsz=640, opset=12, dynamic=True, )  # dynamic=True allows variable batch size
print(f"path to output ONNX model = {output_path}")
