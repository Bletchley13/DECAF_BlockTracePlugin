import os
import sys
import time
import util

FILENAME = "sss.exe"
SAMPLEDIR = "/home/dsns/vmmanage/samples/"
LOG_DIR = "/home/dsns/vmmanage/log/"
MOUNT_POINT = "/tmp/qemu_temp/"
IMAGE_PATH = "/home/dsns/vmmanage/winxp_tmp.img"
ORIGIN_IMAGE_PATH = "/home/dsns/vmmanage/winxp.img"

if __name__ == "__main__":
	
	for filename in os.listdir(SAMPLEDIR):
		util.clean(ORIGIN_IMAGE_PATH, IMAGE_PATH)
		util.mount(IMAGE_PATH, MOUNT_POINT)
		#umount(mount_point)	
		fullpath = os.path.abspath(SAMPLEDIR+filename)
		#print fullpath
		util.insertFile(fullpath, MOUNT_POINT+"/"+FILENAME)
		util.umount(MOUNT_POINT)
		filename = os.path.basename(fullpath)
		time.sleep(1)
		util.umount(MOUNT_POINT)
		util.startAnalysis()
		
		util.mount(IMAGE_PATH, MOUNT_POINT)
		util.getFile(MOUNT_POINT, LOG_DIR+filename+".log")
		util.umount(MOUNT_POINT)
		#deleteFile(filename, mount_point)
	
		util.saveReport(filename, LOG_DIR)
		
		


