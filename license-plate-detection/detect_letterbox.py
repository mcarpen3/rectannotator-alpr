from ultralytics.data.augment import LetterBox
import cv2
import numpy as np
from torchvision import transforms
import torch
from os.path import basename, splitext

# Load image and convert to RGB
img_path = "e:/frame_0044.png"
img_base = basename(img_path)
img = cv2.imread(img_path)
img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

# Wrap in dict, but remove label processing after calling
letterbox = LetterBox(new_shape=(640, 640), auto=False)

# Prepare dict input for letterbox
sample = {"img": img_rgb}  # only the image

# Monkey-patch to skip bbox conversion
letterbox._update_labels = lambda *a, **kw: a[0]  # 🩹 bypass bbox logic

# Apply letterbox
processed = letterbox(sample)
img_lb = processed["img"]  # shape: (640, 640, 3)

cv2.imshow("yolo_preprocessed", cv2.cvtColor(img_lb, cv2.COLOR_RGB2BGR))
cv2.imwrite(f"letterboxed_{splitext(img_base)[0]}.png", cv2.cvtColor(img_lb, cv2.COLOR_RGB2BGR))
cv2.waitKey(0)
cv2.destroyAllWindows()
