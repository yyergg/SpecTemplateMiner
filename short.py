import os
import sys
import shutil
import random
import time
import codecs
import xml.etree.ElementTree as ET


global counterActivity


def compareXML(file1, file2):
    f1 = codecs.open(file1, 'r', "utf-8")
    f2 = codecs.open(file2, 'r', "utf-8")
    if(f1.read() == f2.read()):
        f1.close()
        f2.close()
        return False
    f1.close()
    f2.close()
    return True


def getViews(root):
    returnList = []
    for child in root:
        returnList.append(child)
        returnList.extend(getViews(child))
    return returnList


def ripping(root, tracefilename):
    viewList = getViews(root)
    coordinateList = []
    for view in viewList:
        if view.attrib["clickable"] == "true":
            print(view.attrib["text"], view.attrib["bounds"])
            leftBound = int(
                view.attrib["bounds"].replace("[", "").split("]")[0].split(",")[0])
            topBound = int(
                view.attrib["bounds"].replace("[", "").split("]")[0].split(",")[1])
            rightBound = int(
                view.attrib["bounds"].replace("[", "").split("]")[1].split(",")[0])
            bottomBound = int(
                view.attrib["bounds"].replace("[", "").split("]")[1].split(",")[1])
            coordinateList.append(
                (((leftBound + rightBound) / 2, (topBound + bottomBound) / 2), view))
    target = random.randint(0, len(coordinateList) - 1)
    print(tracefilename)
    f = codecs.open(tracefilename, 'a', "utf-8")
    f.write("CLICK:" + str(list(view.attrib)) + "\n")
    f.close()
    os.system("adb shell input tap " +
              str(coordinateList[target][0][0]) + " " + str(coordinateList[target][0][1]))
    time.sleep(3)


def generateTrace(traceLength, packageName, tracefilename):
    global counterActivity
    i = 0
    
    while i < traceLength:
        print("counterActivity", counterActivity)
        # read in old log
        os.system("adb logcat -d AndroidRuntime:E " +
                  packageName + ":D *:S > templog.txt")
        logfile = open("templog.txt", 'r')
        form = logfile.read()
        logfile.close()

        os.system("adb shell /system/bin/uiautomator dump /data/aaa.xml")
        os.system("adb pull /data/aaa.xml ")
        tree = ET.parse('aaa.xml')
        root = tree.getroot()
        ripping(root, tracefilename)

        # read in new log
        os.system("adb logcat -d AndroidRuntime:E " +
                  packageName + ":D *:S > templog.txt")
        logfile = open("templog.txt", 'r')
        form2 = logfile.read()
        logfile.close()

        if form2.replace(form, "") != "":
            tracefile.write("Error" + form2 + "\n")
            tracefile.close()
            return 0
        else:
            if not os.path.isdir("Activities"):
                os.mkdir("Activities")
            if counterActivity == 0:
                tracefile = open(tracefilename, 'a')
                tracefile.write(os.path.join(os.path.dirname(
                    __file__), "Activities/state_" + str(counterActivity) + ".xml") + "\n")
                tracefile.close()
                shutil.move(
                    "aaa.xml", os.path.join(os.path.dirname(__file__), "Activities/state_" + str(counterActivity) + ".xml"))
                counterActivity = counterActivity + 1
            else:
                newActivity = True
                for root, dirnames, filenames in os.walk("Activities"):
                    for filename in filenames:
                        if(not compareXML(os.path.join(root, filename), "aaa.xml")):
                            tracefile = open(tracefilename, 'a')
                            tracefile.write(
                                os.path.join(root, filename) + "aaa\n")
                            tracefile.close()
                            newActivity = False
                            break
                if newActivity:
                    tracefile = open(tracefilename, 'a')
                    tracefile.write(os.path.join(os.path.dirname(
                        __file__), "Activities/state_" + str(counterActivity) + ".xml") + "\n")
                    tracefile.close()
                    shutil.move(
                        "aaa.xml", os.path.join(os.path.dirname(__file__), "Activities/state_" + str(counterActivity) + ".xml"))
                    counterActivity = counterActivity + 1
        i = i + 1
    tracefile = open(tracefilename, 'a')
    tracefile.write("error free!!")
    tracefile.close()


def main():
    global counterActivity
    packageName = "edu.nyu.cs.omnidroid.app"
    activityName = ".view.simple.ActivityMain"
    os.system("adb root")
    os.system("adb shell am force-stop " + packageName)
    os.system("adb shell am start " + packageName + "/" + activityName)
    counterActivity = 0
    i = 0
    while i < 1:
        os.system("adb shell am force-stop " + packageName)
        os.system("adb shell am start " + packageName + "/" + activityName)
        generateTrace(4, packageName, "trace" + str(i) + ".txt")
        i = i + 1


if __name__ == "__main__":
    main()
