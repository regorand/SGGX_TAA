import sys
import os
import time
from subprocess import Popen, PIPE, STDOUT
import subprocess


def main():    
    seq = 0
    IMG_OUT_DIR = 'tmp_frames'
    VIDEO_DIR = 'videos'
    # TODO better video name
    VIDEO_NAME = 'video' 
    framerate = 60

    C1 = f"ffmpeg -y -r {framerate} -start_number {seq} -i {IMG_OUT_DIR}\\out_%4d_.png -pix_fmt yuv420p -vf pad='width=ceil(iw/2)*2:height=ceil(ih/2)*2' -vb 80M {VIDEO_DIR}\\{VIDEO_NAME}.mp4"
    # C1 = f"ffmpeg -y -r 24 -start_number {seq} -i {IMG_OUT_DIR}\\out_%3d_.png -pix_fmt yuv420p -vb 80M {VIDEO_DIR}\\{VIDEO_NAME}.mp4"
    
    print(C1)
    
    subprocess.call(C1, shell=True)
	
	
if __name__ == "__main__":
	main()