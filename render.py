import csv
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def draw_aabb(ax, min_pt, max_pt):
    xmin, ymin, zmin = min_pt
    xmax, ymax, zmax = max_pt

    # 8 corners of the box
    corners = [
        (xmin, ymin, zmin),
        (xmax, ymin, zmin),
        (xmax, ymax, zmin),
        (xmin, ymax, zmin),
        (xmin, ymin, zmax),
        (xmax, ymin, zmax),
        (xmax, ymax, zmax),
        (xmin, ymax, zmax),
    ]

    # Edges between corners
    edges = [
        (0,1), (1,2), (2,3), (3,0),  # bottom
        (4,5), (5,6), (6,7), (7,4),  # top
        (0,4), (1,5), (2,6), (3,7)   # verticals
    ]

    for i, j in edges:
        x = [corners[i][0], corners[j][0]]
        y = [corners[i][1], corners[j][1]]
        z = [corners[i][2], corners[j][2]]
        ax.plot(x, y, z, color="black")


# ---- Load CSV ----
aabbs = []
with open("bvh.csv", newline="") as f:
    reader = csv.DictReader(f)
    for row in reader:
        min_pt = (
            float(row["MinX"]),
            float(row["MinY"]),
            float(row["MinZ"]),
        )
        max_pt = (
            float(row["MaxX"]),
            float(row["MaxY"]),
            float(row["MaxZ"]),
        )
        child_index = int(row["ChildIndex"])
        triangle_count = int(row["TriangleCount"])
        if child_index == 0 and triangle_count > 0:
            aabbs.append((min_pt, max_pt))


# ---- Render ----
fig = plt.figure()
ax = fig.add_subplot(111, projection="3d")

for min_pt, max_pt in aabbs:
    draw_aabb(ax, min_pt, max_pt)

ax.set_xlabel("X")
ax.set_ylabel("Y")
ax.set_zlabel("Z")
ax.set_box_aspect([1, 1, 1])  # equal scale

plt.show()
