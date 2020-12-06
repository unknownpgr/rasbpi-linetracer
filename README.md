# Raspberry Pi-Based Lintracer
This is a source code for Raspberry Pi line tracer, which is a practical task in the course `Embedded System Design`. This tracer detects yellow line on black background and follow it. The algorithm is as follows.
1. Resize image (1/4)
2. Convert the image into HSV
3. Detect lane with Hue range filter
4. Calculate lane position from weighted some of detected lane pixel
5. Calculate wheel speed
6. Drive!
---