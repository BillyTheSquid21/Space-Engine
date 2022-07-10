import os;
from PIL import Image;
from PIL.ExifTags import TAGS;

img_file = 'C:\\Users\\yoshi\\Downloads\\JPEG_example_flower.jpg'
image = Image.open(img_file);

for tag, value in image._getexif().items():
    print(tag, value)


