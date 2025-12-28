# Fourier Animator

A real-time visualization tool that demonstrates the Discrete Fourier Transform (DFT) through animated epicycles in pure C. Draw any shape and watch it being reconstructed by rotating circles!. 

![C](https://img.shields.io/badge/C-00599C?style=flat&logo=c&logoColor=white)
![Raylib](https://img.shields.io/badge/Raylib-5.5-black?style=flat)
![Platform](https://img.shields.io/badge/Platform-Windows-blue?style=flat)
![License](https://img.shields.io/badge/License-MIT-green?style=flat)

## ✨ Features

- **Freehand Drawing** – Draw any shape with your mouse and see it transformed
- **Epicycle Visualization** – Watch rotating circles (epicycles) reconstruct your drawing in real-time
- **Preset Shapes** – Quickly load geometric shapes: Circle, Square, Star, Heart, Infinity, Spiral
- **SVG Support** – Import complex vector graphics from SVG files
- **Drag & Drop** – Drop SVG or TXT files directly onto the window
- **Interactive Controls**
  - Adjustable animation speed (0.1x – 5x)
  - Line thickness customization
  - Line color selection (6 presets)
  - Real-time Fourier coefficient display
- **Clean UI** – Modern dark theme with intuitive controls


## 🔧 Requirements

- **Compiler**: MSVC (Visual Studio Build Tools) or compatible C compiler
- **Graphics Library**: [raylib 5.5](https://www.raylib.com/)
- **Platform**: Windows (uses Windows-specific file APIs)

## 🚀 Building

### Prerequisites

1. Install [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/) with C++ workload
2. Raylib 5.5 is included in the `libs/` and `includes/` directories

### Compile & Run
```bash
python build.py
```

Or compile manually:

```bash
cl /W4 /WX /Iincludes src/main.c src/fourier.c src/shapes.c src/ui.c libs/raylibdll.lib
main.exe
```

## 📖 Usage

### Drawing Mode
1. Click and drag anywhere on the canvas to draw a shape
2. Release the mouse to start the Fourier animation
3. Watch the epicycles reconstruct your drawing!

### Preset Shapes
- Click any shape button (Circle, Square, Star, etc.) to generate a perfect geometric shape

### Loading Custom Shapes

**From shapes folder:**
- Click "Browse Files..." to see available shapes in the `shapes/` directory

**Drag & Drop:**
- Drag any `.svg` or `.txt` file onto the window from anywhere on your PC

### Controls
| Control | Description |
|---------|-------------|
| **Speed Slider** | Adjust animation speed (0.1x – 5x) |
| **Line Size Slider** | Change trace line thickness |
| **Line Color** | Pick from 6 color presets (Fire, Ocean, Lime, Purple, Gold, Cyan) |
| **RESTART** | Reset and draw a new shape |

## 📁 Supported File Formats

### TXT Format
Simple coordinate list with one point per line:
```
x1 y1
x2 y2
x3 y3
...
```

Example (`shapes/shape.txt`):
```
0 -100
0 50
-30 50
-30 100
30 100
30 50
```

### SVG Format
Standard SVG files with path elements. Supported path commands:
- `M/m` – Move to
- `L/l` – Line to
- `H/h` – Horizontal line
- `V/v` – Vertical line
- `C/c` – Cubic Bézier curve
- `S/s` – Smooth cubic Bézier
- `Q/q` – Quadratic Bézier curve
- `Z/z` – Close path


## 🎓 How It Works

The **Discrete Fourier Transform** decomposes any signal into a sum of sinusoids at different frequencies. In 2D:

1. **Input**: A series of (x, y) points representing a drawn path
2. **Transform**: Each point is treated as a complex number (x + iy), and DFT computes frequency components
3. **Epicycles**: Each frequency component becomes a rotating circle (epicycle) with:
   - **Radius** = amplitude of the component
   - **Speed** = frequency index
   - **Starting angle** = phase of the component
4. **Reconstruction**: Stacking all rotating circles tip-to-tail traces the original drawing

The more points (N) in your drawing, the more epicycles, and the more accurate the reconstruction.

### The Math

For N sample points, the DFT is:

$$X[k] = \frac{1}{N} \sum_{n=0}^{N-1} x[n] \cdot e^{-i2\pi kn/N}$$

Each $X[k]$ gives us an epicycle with:
- Amplitude: $|X[k]|$
- Phase: $\arg(X[k])$
- Frequency: $k$


## 🤝 Wanna Help?

Hey, if you find this cool or useful, feel free to mess around with it! You can:

- Fork it and make it your own
- Open a PR if you added something nice
- Drop an issue if something's broken
- Add your own shapes to the `shapes/` folder
- Or just use it to learn – that's what it's here for!

No pressure, no rules. Just have fun with it 🎉

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [raylib](https://www.raylib.com/) – Simple and easy-to-use graphics library
- [3Blue1Brown](https://www.youtube.com/watch?v=r6sGWTCMz2k) – Inspiration for Fourier visualization
- [My Fourier Epicycles](https://www.myfourierepicycles.com/) – Some SVG sample files

---