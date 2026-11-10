"""Tách tài nguyên giao diện đăng nhập từ ảnh thiết kế (1578x996)."""
import sys, numpy as np, cv2
from PIL import Image, ImageDraw
SRC, OUT = sys.argv[1], sys.argv[2]
im = np.array(Image.open(SRC).convert("RGB"))
H, W = im.shape[:2]

# ---- 1. nền: xoá chữ/logo/tile bên trái + vùng thẻ đăng nhập ----
mask = np.zeros((H, W), np.uint8)
for x0, y0, x1, y1 in [(122, 78, 692, 218), (122, 238, 600, 322), (120, 338, 702, 644), (826, 74, 1522, 950)]:
    mask[y0:y1, x0:x1] = 255
bg = cv2.inpaint(cv2.cvtColor(im, cv2.COLOR_RGB2BGR), mask, 5, cv2.INPAINT_TELEA)
bg = cv2.cvtColor(bg, cv2.COLOR_BGR2RGB)
bg = cv2.GaussianBlur(bg, (0, 0), 1.2) * 0 + bg  # giữ nguyên
Image.fromarray(bg).save(f"{OUT}/login_bg.png")

def cut_alpha(box, name, ref=bg):
    x0, y0, x1, y1 = box
    a = im[y0:y1, x0:x1].astype(np.float32); r = ref[y0:y1, x0:x1].astype(np.float32)
    d = np.abs(a - r).max(axis=2)
    al = np.clip((d - 3) / 20.0, 0, 1)
    al[al < 0.08] = 0
    safe = np.maximum(al, 0.15)[..., None]
    col = np.clip(r + (a - r) / safe, 0, 255)
    rgba = np.dstack([col, al * 255]).astype(np.uint8)
    Image.fromarray(rgba, "RGBA").save(f"{OUT}/{name}.png")

def cut_plain(box, name):      # icon trên nền thẻ trắng
    x0, y0, x1, y1 = box
    Image.fromarray(im[y0:y1, x0:x1]).save(f"{OUT}/{name}.png")

cut_alpha((138, 82, 290, 202), "logo_cap")
def cut_tile(search, name):
    x0, y0, x1, y1 = search
    d = np.abs(im[y0:y1, x0:x1].astype(int) - bg[y0:y1, x0:x1].astype(int)).max(axis=2) > 14
    ys, xs = np.where(d)
    bx0, bx1, by0, by1 = xs.min() + x0, xs.max() + x0 + 1, ys.min() + y0, ys.max() + y0 + 1
    w, h = bx1 - bx0, by1 - by0
    S = 8
    m = Image.new("L", (w * S, h * S), 0)
    ImageDraw.Draw(m).rounded_rectangle((0.7 * S, 0.7 * S, (w - 0.7) * S - 1, (h - 0.7) * S - 1), radius=0.29 * w * S, fill=255)
    m = m.resize((w, h), Image.LANCZOS)
    rgb = Image.fromarray(im[by0:by1, bx0:bx1])
    rgb.putalpha(m)
    rgb.save(f"{OUT}/{name}.png")
    print(name, "bbox", bx0, by0, w, h)
cut_tile((122, 338, 226, 440), "tile_learn")
cut_tile((122, 441, 226, 543), "tile_progress")
cut_tile((122, 544, 226, 646), "tile_connect")
cut_plain((892, 246, 938, 296), "icon_user")
cut_plain((892, 378, 938, 424), "icon_lock")
cut_plain((1062, 758, 1098, 794), "icon_edu")

# ---- 2. icon vẽ bằng code (siêu lấy mẫu) ----
S = 8
def canvas(w, h): return Image.new("RGBA", (w*S, h*S), (0, 0, 0, 0))
def done(img, w, h, name): img.resize((w, h), Image.LANCZOS).save(f"{OUT}/{name}.png")

slate = (96, 114, 153, 255)
def eye(slash, name):
    w, h = 64, 64; c = canvas(w, h); d = ImageDraw.Draw(c)
    top = []; bot = []
    for i in range(61):
        t = i / 60; x = 6 + 52 * t; k = np.sin(np.pi * t) ** 0.85
        top.append((x * S, (32 - 19 * k) * S)); bot.append((x * S, (32 + 19 * k) * S))
    d.polygon(top + bot[::-1], fill=slate)
    d.ellipse((32*S-10.5*S, 32*S-10.5*S, 32*S+10.5*S, 32*S+10.5*S), fill=(255, 255, 255, 255))
    d.ellipse((32*S-5.2*S, 32*S-5.2*S, 32*S+5.2*S, 32*S+5.2*S), fill=slate)
    if slash:
        d.line((13*S, 51*S, 51*S, 13*S), fill=(255, 255, 255, 255), width=int(11*S))
        d.line((13*S, 51*S, 51*S, 13*S), fill=slate, width=int(4.6*S))
    done(c, w, h, name)
eye(True, "eye_off"); eye(False, "eye_on")

c = canvas(40, 40); d = ImageDraw.Draw(c)
d.line([(10*S, 21*S), (17*S, 28*S), (30*S, 12*S)], fill=(255, 255, 255, 255), width=int(4.2*S), joint="curve")
for p in [(10, 21), (17, 28), (30, 12)]:
    r = 2.1*S; d.ellipse((p[0]*S-r, p[1]*S-r, p[0]*S+r, p[1]*S+r), fill=(255, 255, 255, 255))
done(c, 40, 40, "check")
print("ok", W, H)
