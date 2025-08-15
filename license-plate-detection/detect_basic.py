from ultralytics import YOLO
import cv2
import os
import numpy as np

# --- config ---
weights_path = "runs/detect/train6/weights/best.pt"
image_path   = "dataset/images2/2AE1A25C7E20250811153004051_FF1_0.jpg"
conf_thresh  = 0.25          # adjust as desired
pick         = "best"        # "best" or "all"

# --- load ---
model = YOLO(weights_path)
img = cv2.imread(image_path)
assert img is not None, f"Could not read {image_path}"

# --- infer ---
results = model.predict(img, conf=conf_thresh, verbose=False)
r = results[0]

# optional: view annotated image
annotated = r.plot()

# --- find class id(s) for license plates ---
names = model.names  # dict: {cls_id: "name"}
print(f"model.names: {names}")
plate_cls_ids = {i for i, n in names.items() if "plate" in n.lower()}
if not plate_cls_ids and len(names) == 1:
    plate_cls_ids = {0}  # single-class model

boxes = r.boxes  # ultralytics.engine.results.Boxes
print(f"boxes ")
H, W = img.shape[:2]
crops = []  # (crop, (x1,y1,x2,y2), score)

if boxes is not None and len(boxes) > 0:
    xyxy = boxes.xyxy.cpu().numpy()
    conf = boxes.conf.cpu().numpy()
    cls  = boxes.cls.cpu().numpy().astype(int)

    for (x1, y1, x2, y2), score, cid in zip(xyxy, conf, cls):
        if cid in plate_cls_ids:
            # clamp to image bounds
            x1 = int(max(0, min(W - 1, x1)))
            y1 = int(max(0, min(H - 1, y1)))
            x2 = int(max(0, min(W,     x2)))
            y2 = int(max(0, min(H,     y2)))
            if x2 > x1 and y2 > y1:
                crop = img[y1:y2, x1:x2].copy()
                crops.append((crop, (x1, y1, x2, y2), float(score)))

# --- keep only best if requested ---
if pick == "best" and crops:
    crops = [max(crops, key=lambda t: t[2])]

# --- save crops ---
base, _ = os.path.splitext(image_path)
for i, (crop, (x1, y1, x2, y2), score) in enumerate(crops):
    out_path = f"{base}_plate{i}_{score:.2f}.png"
    cv2.imwrite(out_path, crop)
    print(f"Saved {out_path}  bbox={(x1,y1,x2,y2)}  score={score:.3f}")

# --- show annotated preview (optional) ---
cv2.imshow("YOLOv8 Detection", annotated)
cv2.waitKey(0)               # <-- important so the window shows
cv2.destroyAllWindows()
