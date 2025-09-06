# live_view.py
import sys, matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import numpy as np

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
plt.ion()

while True:
    line = sys.stdin.readline()
    if not line: break
    vals = list(map(float, line.strip().split(',')))
    if len(vals) != 11: continue
    nx,ny,nz,b, px,py,pz, qx,qy,qz, phi = vals
    ax.cla()
    ax.set_xlim(-2,2); ax.set_ylim(-2,2); ax.set_zlim(-2,2)
    ax.set_xlabel('X'); ax.set_ylabel('Y'); ax.set_zlabel('Z')
    n = np.array([nx,ny,nz])
    ref = np.array([0,1,0]) if abs(n[1])<0.9 else np.array([1,0,0])
    u = np.cross(n, ref); u /= np.linalg.norm(u)
    v = np.cross(n, u)
    s = 2
    center = n * b
    corners = [center + (a*u + b2*v)*s for a,b2 in [(1,1),(1,-1),(-1,-1),(-1,1)]]
    poly = Poly3DCollection([corners], facecolors=(0.3,0.6,0.9,0.3), edgecolors='k', linewidths=0.5)
    ax.add_collection3d(poly)
    color = 'red' if phi < 0 else 'green'
    ax.scatter([px],[py],[pz], c=color, s=40)
    ax.scatter([qx],[qy],[qz], c='white', s=30)
    ax.plot([px,qx],[py,qy],[pz,qz], color='yellow')
    ax.set_title(f'phi={phi:.3f}')
    plt.pause(0.001)