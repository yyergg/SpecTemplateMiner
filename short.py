import os
import sys
import shutil
import xml.etree.ElementTree as ET
#adb shell input tap x y



def getViews(root):
    for child in root:
        print(child.attrib["class"])
        getViews(child)



def ripping(root):
    print("")
    #now click a constaint coordinate
    #todo:
    #1. capture clickable views in root
    #2. calculate which one to click and the coordinate
    viewList=getViews(root)





def generateTrace(traceLength,packageName,folder):
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
    while i<1:
        generateTrace(1,packageName,"trace_"+str(i))
        i=i+1

if __name__ == "__main__":
    main()

#notes
#packageName=edu.nyu.cs.omnidroid.app
#activityName=.view.simple.ActivityMain
#cmp=edu.nyu.cs.omnidroid.app/.view.simple.ActivityMain
#adb logcat AndroidRuntime:E edu.nyu.cs.omnidroid.app:D *:S