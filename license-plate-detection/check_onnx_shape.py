import onnx
m = onnx.load("runs/detect/train8/weights/best.onnx")
print(m.graph.output[0].type.tensor_type.shape)
