# Custom Shape File Format
# ========================
# 
# Each line contains: x y (space-separated coordinates)
# Lines starting with # are comments (ignored)
# The shape will be automatically centered and scaled
#
# TIPS FOR CREATING SHAPES:
# -------------------------
# 
# 1. HAND-DRAW: Use any drawing app, trace outline, note coordinates
#
# 2. SVG PATH: Use online tools to convert SVG paths to points:
#    - https://shinao.github.io/PathToPoints/
#    - Export SVG from Inkscape/Illustrator, paste path, get points
#
# 3. IMAGE TRACING: Use edge detection tools:
#    - Potrace (command line)
#    - Online image-to-SVG converters, then SVG to points
#
# 4. PROGRAMMATIC: Generate points with Python/JS:
#    ```python
#    import math
#    for i in range(100):
#        t = i / 100 * 2 * math.pi
#        x = math.cos(t) * 100  # your parametric equation
#        y = math.sin(t) * 100
#        print(f"{x:.1f} {y:.1f}")
#    ```
#
# EXAMPLE - Simple Triangle:
# 0 -100
# 87 50
# -87 50
# 0 -100
#
# Place your custom shapes in this folder as .txt files
# Default loaded file: shape.txt
