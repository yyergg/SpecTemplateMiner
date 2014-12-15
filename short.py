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


def ripping(root, tracefilename, stateIndex):
    viewList = getViews(root)
    coordinateList = []
    indexs=[]
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
            indexs.append(view.attrib["index"])
    target = random.randint(0, len(coordinateList) - 1)
    f = codecs.open(tracefilename, 'a', "utf-8")
    f.write("CLICK:" + indexs[target] + "\n")
    f.close()
    f = codecs.open("traces", 'a', "utf-8")
    f.write(" "+stateIndex+ "_" + indexs[target])
    f.close()
    os.system("adb shell input tap " +
              str(coordinateList[target][0][0]) + " " + str(coordinateList[target][0][1]))
    time.sleep(1)


def findMatchState():
    global counterActivity
    for root, dirnames, filenames in os.walk("Activities"):
        for filename in filenames:
            if(not compareXML(os.path.join(root, filename), "aaa.xml")):
                return os.path.join(os.path.dirname(__file__), "Activities/" + filename) + "(match)"
    newfile = os.path.join(
        os.path.dirname(__file__), "Activities/state_" + str(counterActivity) + ".xml")
    shutil.copyfile("aaa.xml", newfile)
    counterActivity = counterActivity + 1
    return newfile + "(new)"


def generateTrace(traceLength, packageName, tracefilename):
    global counterActivity

    i = 0

    while i < traceLength:
        # read in old log
        os.system("adb logcat -d AndroidRuntime:E " +
                  packageName + ":D *:S > templog.txt")
        logfile = open("templog.txt", 'r')
        form = logfile.read()
        logfile.close()

        os.system("adb shell /system/bin/uiautomator dump /data/aaa.xml")
        os.system("adb pull /data/aaa.xml ")
        if i == 0:
            statename = findMatchState()
            tracefile = open(tracefilename, 'w')
            tracefile.write(statename + "\n")
            tracefile.close()
            tracefiles = codecs.open("traces", 'a', "utf-8")
            tracefiles.write("\n" + statename.split("_")[-1].split("(")[0].replace(".xml", ""))
            tracefiles.close()

        tree = ET.parse('aaa.xml')
        root = tree.getroot()
        ripping(root, tracefilename, statename.split("_")[-1].split("(")[0].replace(".xml", ""))

        # read in new log
        os.system("adb logcat -d AndroidRuntime:E " +
                  packageName + ":D *:S > templog.txt")
        logfile = open("templog.txt", 'r')
        form2 = logfile.read()
        logfile.close()

        if form2.replace(form, "") != "":
            tracefile = open(tracefilename, 'a')
            tracefile.write("Error" + form2 + "\n")
            tracefile.close()
            tracefiles = codecs.open("traces", 'a', "utf-8")
            tracefiles.write(" fail")
            tracefiles.close()
            return 0
        else:
            os.system("adb shell /system/bin/uiautomator dump /data/aaa.xml")
            os.system("adb pull /data/aaa.xml ")
            statename = findMatchState()
            tracefile = open(tracefilename, 'a')
            tracefile.write(statename + "\n")
            tracefile.close()
            tracefiles = codecs.open("traces", 'a', "utf-8")
            tracefiles.write(" " + statename.split("_")[-1].split("(")[0].replace(".xml", ""))
            tracefiles.close()
        i = i + 1
    tracefile = open(tracefilename, 'a')
    tracefile.write("error free!!")
    tracefile.close()
    tracefiles = codecs.open("traces", 'a', "utf-8")
    tracefiles.write(" pass")
    tracefiles.close()


def main():
    global counterActivity
    packageName = "edu.nyu.cs.omnidroid.app"
    activityName = ".view.simple.ActivityMain"
    os.system("adb root")
    os.system("adb shell am force-stop " + packageName)
    os.system("adb shell am start " + packageName + "/" + activityName)
    counterActivity = 0
    i = 0
    if not os.path.isdir("Activities"):
        os.mkdir("Activities")
    while i < 20:
        os.system("adb shell am force-stop " + packageName)
        os.system("adb shell am start " + packageName + "/" + activityName)
        generateTrace(20, packageName, "trace" + str(i) + ".txt")
        i = i + 1


if __name__ == "__main__":
    main()
