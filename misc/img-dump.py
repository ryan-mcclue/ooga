#!/usr/bin/python3
# SPDX-License-Identifier: zlib-acknowledgement

from PIL import Image

# Load the BMP image
bmp_image_path = 'Untitled.bmp'
img = Image.open(bmp_image_path)

# Ensure the image is in RGB mode
img = img.convert('RGB')

# Convert the image to raw byte data
byte_data = list(img.tobytes())

# Group the bytes into RGB triplets
rgb_data = [byte_data[i:i+3] for i in range(0, len(byte_data), 3)]

# Format the data as u32 in 0xRRGGBB format
formatted_data = [f"0x{rgb[0]:02X}{rgb[1]:02X}{rgb[2]:02X}" for rgb in rgb_data]

# Group the formatted data into lines with 16 values per line
lines = [", ".join(formatted_data[i:i+16]) for i in range(0, len(formatted_data), 16)]

# Join all lines with a newline to form the final output
output_string = ",\n".join(lines)

# Print the formatted output string
print(f"u32 data[] = {{\n{output_string}\n}};")
