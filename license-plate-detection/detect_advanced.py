import cv2
import numpy as np
from ultralytics.models.yolo.detect import DetectionPredictor
from ultralytics.cfg import get_cfg
from ultralytics.utils import DEFAULT_CFG

# Step 1: Setup predictor properly
args = get_cfg(DEFAULT_CFG)
args.model = "runs/detect/train6/weights/best.pt"
args.source = "e:/frame_0044.png"   # ✅ Needed for setup_source()
args.imgsz = 640                    # ✅ Required
predictor = DetectionPredictor(overrides=args)
predictor.setup_model(args.model)
predictor.setup_source(args.source)  # ✅ THIS sets self.imgsz and self.source correctly

# Step 2: Now call preprocess — it will use the loaded source image properly
batch = predictor.preprocess(predictor.source)

# Step 3: Save the preprocessed image for comparison
chw = batch[0].cpu().numpy()  # shape: (3, 640, 640)
hwc = np.transpose(chw, (1, 2, 0)) * 255
hwc = hwc.astype(np.uint8)

import cv2
cv2.imwrite("yolo_preprocessed.png", cv2.cvtColor(hwc, cv2.COLOR_RGB2BGR))
cv2.imshow("YOLOv8 Preprocessed Input", hwc[:, :, ::-1])
cv2.waitKey(0)
cv2.destroyAllWindows()
