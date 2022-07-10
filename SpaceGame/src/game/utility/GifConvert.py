from os import listdir
from os.path import isfile, join
from PIL import Image
from PIL import GifImagePlugin
from PIL.PngImagePlugin import PngInfo
from dataclasses import dataclass

#Script to turn gif files (eg from the pokemon showdown archive) into compatible pngs
#Turns the gif into an image strip, with a saved metadata tag with the original frame count

PATH_TO_GIF_FOLDER = "C:\\Users\\yoshi\\Downloads\\test\\gifs\\"
PATH_TO_PNG_FOLDER = "C:\\Users\\yoshi\\Downloads\\test\\pngs\\"
SHOW_PROGRESS = True

@dataclass
class CreatedImage:
    image : Image
    frames : int = 0                    

#Get all files in the gif directory
def get_Directory_Paths(path):
    files = [f for f in listdir(path) if isfile(join(path, f))]
    return files;

#Get the frames and process
def export_Gif_Frames(pathToGif, pathToPng):
    files = get_Directory_Paths(pathToGif)
    for file in range(0,len(files)):
        gifPath = pathToGif+files[file]
        createdImg = get_Gif_Frames(gifPath)
        pngName = files[file].split('.')[0]
        pngPath = pathToPng+pngName+".png"
        save_Final_Image(createdImg.frames, createdImg.image, pngPath)
        if (SHOW_PROGRESS):
            print("Image " + str(file+1) + " of " + str(len(files)))

#Gets frames from gif file with count
def get_Gif_Frames(path):
    #Opens image as gif
    image = Image.open(path)
    
    #Strip frames into an array
    imageArray = []
    for frame in range(0,image.n_frames):
        image.seek(frame)
        imageArray.append(image.copy())

    #Stitch images together
    img = merge_images(imageArray, image.n_frames)
    return img;
    
#Merge images
def merge_images(images, frames):

    #Get width and height of image
    width = images[0].width
    result_width = width * frames
    result_height = images[0].height

    #Allocate result
    result = Image.new('RGBA', (result_width, result_height))

    #Stitch together then combine
    for frame in range(0, frames):
        result.paste(im=images[frame], box=(width*frame, 0))
        
    combined = CreatedImage(result, frames)
    
    return combined

#Adds frame count as metadata to the image
def save_Final_Image(framecount, targetImage, path):
    #Add metadata expressing frame count
    metadata = PngInfo()
    metadata.add_text("frames", str(framecount))

    #Save with new metadata
    targetImage.save(path, pnginfo=metadata)

#Main
def main():
    print("Starting process...")
    export_Gif_Frames(PATH_TO_GIF_FOLDER, PATH_TO_PNG_FOLDER)

if __name__ == "__main__":
    main()
