import os
import sys
import shutil
import random
import time
import xml.etree.ElementTree as ET




def getViews(root):
    returnList=[]
    for child in root:
        returnList.append(child)
        returnList.extend(getViews(child))
    return returnList



def ripping(root):
    print("")
    viewList=getViews(root)
    coordinateList=[]
    for view in viewList:
        if view.attrib["clickable"]=="true":
            print(view.attrib["text"],view.attrib["bounds"])
            leftBound=int(view.attrib["bounds"].replace("[","").split("]")[0].split(",")[0])
            topBound=int(view.attrib["bounds"].replace("[","").split("]")[0].split(",")[1])
            rightBound=int(view.attrib["bounds"].replace("[","").split("]")[1].split(",")[0])
            bottomBound=int(view.attrib["bounds"].replace("[","").split("]")[1].split(",")[1])
            coordinateList.append(((leftBound+rightBound)/2,(topBound+bottomBound)/2))
    target=random.randint(0,len(coordinateList)-1)
    os.system("adb shell input tap "+str(coordinateList[target][0])+" "+str(coordinateList[target][1]))
    time.sleep(3)



def generateTrace(traceLength,packageName,folder):
    if(os.path.isdir(folder)):
        shutil.rmtree(folder)
    os.mkdir(folder)
    i=0
    while i<traceLength:
        os.system("adb logcat -d AndroidRuntime:E "+packageName+":D *:S > templog.txt")
        logfile=open("templog.txt",'r')
        form=logfile.read()
        logfile.close()

        os.system("adb shell /system/bin/uiautomator dump /data/aaa.xml")
        os.system("adb pull /data/aaa.xml ")
        tree = ET.parse('aaa.xml')
        root = tree.getroot()
        ripping(root)

        os.system("adb logcat -d AndroidRuntime:E "+packageName+":D *:S > templog.txt")
        logfile=open("templog.txt",'r')
        form2=logfile.read()
        logfile.close()
        if form2.replace(form,"")!="":
            errorfile=open(os.path.join(folder,"error.txt"),'w')
            errorfile.write(form2)
            errorfile.close()
            return 0
        else:
            shutil.move("aaa.xml", os.path.join(folder,"state_"+str(i)+".xml"))
            i=i+1
    errorfile=open(os.path.join(folder,"error.txt"),'w')
    errorfile.write("error free!!")
    errorfile.close()    

def main():
    packageName="edu.nyu.cs.omnidroid.app"
    activityName=".view.simple.ActivityMain"
    os.system("adb root")
    os.system("adb shell am force-stop "+packageName)
    os.system("adb shell am start "+packageName+"/"+activityName)
    i=0
    while i<3:
        os.system("adb shell am force-stop "+packageName)
        os.system("adb shell am start "+packageName+"/"+activityName)
        generateTrace(5,packageName,"trace"+str(i))
        i=i+1


if __name__ == "__main__":
    main()

