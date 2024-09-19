import time
from picamera2 import Picamera2, Preview
import tflite_runtime.interpreter as tf
import numpy as np
from PIL import Image as im
from PIL import ImageFilter, ImageOps
import pytesseract
from gpiozero import DigitalOutputDevice as dout

def setup():
    global plate, outdictplate, indictplate, vehicle, outdictvehicle, indictvehicle,cam,viplist,pin0,pin1
    plate = tf.Interpreter(model_path="plate.tflite")
    plate.allocate_tensors()
    indictplate = plate.get_input_details()
    outdictplate = plate.get_output_details()
    vehicle = tf.Interpreter(model_path="vehicel.tflite")
    vehicle.allocate_tensors()
    indictvehicle = vehicle.get_input_details()
    outdictvehicle = vehicle.get_output_details()
    pin0=dout(14)
    pin1=dout(15)
    pin0.off()
    pin1.off()
    cam = Picamera2()
    config = cam.create_preview_configuration(main={"size":(1200,800)})
    cam.configure(config)
    cam.set_controls({"Saturation":1,"ExposureTime":80000})
    cam.start()
    time.sleep(2)
    viplist=[]


def captur():
    global img
    img=cam.capture_image("main")
    img=img.convert("RGB")
    
def vehitype():
    label=["other","bike","bus","truck","car","nothing"]
    imgvehi=img.resize((224,224))
    vehiin=[np.asarray(imgvehi,dtype="uint8")]
    
    vehicle.set_tensor(indictvehicle[0]["index"],vehiin)
    vehicle.invoke()
    vehicletype=vehicle.get_tensor(outdictvehicle[0]["index"])
    #print(max(vehicletype[0])/255)
    return list(vehicletype[0]).index(max(vehicletype[0]))
        
def objbox():
    global boxindex,box
    imgplat=img.resize((512,512))
    platin=[np.asarray(imgplat,dtype="uint8")]
    
    plate.set_tensor(indictplate[0]["index"],platin)
    plate.invoke()
    box=plate.get_tensor(outdictplate[0]["index"])
    platscor=plate.get_tensor(outdictplate[2]["index"])
    
    boxindex=list(platscor[0]).index(max(platscor[0]))
    #print(max(platscor[0]))
    return (box[0][boxindex][1]*512,box[0][boxindex][0]*512,box[0][boxindex][3]*512,box[0][boxindex][2]*512)

def preproc(box):
    scale=3
    procimg=img.resize((512,512))
    procimg=procimg.crop(box)
    procimg=ImageOps.grayscale(procimg)
    procimg=ImageOps.autocontrast(procimg)
    procimg=ImageOps.invert(procimg)
    #procimg.show()
    procimg=procimg.resize((procimg.size[0]*scale,procimg.size[1]*scale))
    return procimg
    
def ocr(textimg):
    return pytesseract.image_to_string(textimg,lang="eng")

def verify(plat):
    for a in viplist:
        correkcount=0
        count=0
        for b in a:
            if plat[count]==b:
                correkcount+=1
            count+=1
        if correkcount/len(a)>=0.6:
            return True
    return False

setup()

 
while 1:
    captur()
    typevehicle=vehitype()
    plat=ocr(preproc(objbox()))
    if(typevehicle==4):
        if(verify(plat)):
             pin0.on()
             pin1.on()
        else:
             pin0.on()
    elif typevehicle==5:
        pin0.off()
        pin1.off()
    else:
        pin1.on()
        
    time.sleep(4)
    pin0.off()
    pin1.off()
    time.sleep(1)
        
             
