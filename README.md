# Image Detector

Scratch library for messing around with opencv. Primary use is to detect the edge of images within images (e.g. crop a meme in a screenshot).

## Example

![example.png](./example.png)

## Getting Started

1. Install opencv: `brew install opencv`
2. Add some images to the `test_images` directory
3. Modify the image paths in `main()` of `main.cpp`
4. Run `make IN="test_images/image.png"` changing the `IN` variable to point at one of your test images.
5. Press `q` to close the windows
