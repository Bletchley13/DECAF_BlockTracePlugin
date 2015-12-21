import os
import sys
import time

FILENAME = "sss.exe"
SAMPLEDIR = "/home/dsns/vmmanage/samples/"
LOG_DIR = "/home/dsns/vmmanage/log/"
MOUNT_POINT = "/tmp/qemu_temp/"
IMAGE_PATH = "/home/dsns/vmmanage/winxp_tmp.img"
ORIGIN_IMAGE_PATH = "/home/dsns/vmmanage/winxp.img"

def mount(image_name, mount_point):

	cmd = "modprobe nbd max_part=8"
	print os.system(cmd)

	cmd = "qemu-nbd --connect=/dev/nbd0 "+image_name
	print os.system(cmd)

	time.sleep(1)

	cmd = "mkdir "+mount_point
	os.system(cmd)

	cmd = "mount -t ntfs /dev/nbd0p1 "+mount_point
	print os.system(cmd)

def umount(mount_point):

	cmd = "umount "+mount_point
	print os.system(cmd)

	cmd = "rm "+mount_point
	print os.system(cmd)

	cmd = "qemu-nbd -d /dev/nbd0"
	print os.system(cmd)

def insertFile(origin, target):
	cmd = "cp "+ origin + " " + target
	print cmd
	print os.system(cmd)

def getFile(image_mount, target):
	cmd = "cp "+ image_mount + "/log " + target
	print os.system(cmd)

def saveReport(filename, logdir):
	cmd = "cp /home/dsns/vmlog/btrace "+logdir+"/"+filename+".btrace"
	print os.system(cmd)


def deleteFile(name, target):
	cmd = "rm -rf "+ target + "/" + name	
	print cmd
	print os.system(cmd)


def startAnalysis():
	cmd = "sh /home/dsns/decaf1.9/startvm.sh"
	print os.system(cmd)

def clean(old_image, image_path):
	cmd = "cp "+old_image+" "+image_path
	print os.system(cmd)
	cmd = "killall -8 qemu-system-i386"
        print os.system(cmd)
	pass



if __name__ == "__main__":
	
	for filename in os.listdir(SAMPLEDIR):
		clean(ORIGIN_IMAGE_PATH, IMAGE_PATH)
		mount(IMAGE_PATH, MOUNT_POINT)
		#umount(mount_point)	
		fullpath = os.path.abspath(SAMPLEDIR+filename)
		#print fullpath
		insertFile(fullpath, MOUNT_POINT+"/"+FILENAME)
		umount(MOUNT_POINT)
		filename = os.path.basename(fullpath)
		time.sleep(1)
		umount(MOUNT_POINT)
		startAnalysis()
		
		mount(IMAGE_PATH, MOUNT_POINT)
		getFile(MOUNT_POINT, LOG_DIR)
		umount(MOUNT_POINT)

		saveReport(filename, LOG_DIR)
		#deleteFile(filename, mount_point)
		
		


