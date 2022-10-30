from ctypes import sizeof
from PIL import Image

startFrame = 0
targetFrame = 650


finalStr = '#AS\n\nasm"\n'

def formatI(x):
    if x < 10:
        return "000" + str(x)
    if x < 100:
        return "00" + str(x)
    if x < 1000:
        return "0" + str(x)
    return str(x)

#for group in range(0, 1):
tmpValList = [ [0]*183 for i in range(targetFrame-startFrame)]

frameCnt = 0
for frame in range(startFrame, targetFrame):
    im = Image.open('./BadApple-Frames/bad_apple_' + formatI(frame) + '.png')
    pix = im.load()
    print(im.size)
    pixel_values = list(im.getdata())
    print(pixel_values[0])

    # 2916
    bitIndex = 0
    byte = 0
    for p in range(0, 54*54):
        pixVal = 1 if pixel_values[p][0] == 255 else 0
        pixVal <<= bitIndex
        tmpValList[frameCnt][byte] |= pixVal

        bitIndex += 1
        if bitIndex == 16:
            bitIndex = 0
            byte += 1

    frameCnt += 1
        #pix[x,y] = tuple([int(pix[x,y][0]/255)*255,int(pix[x,y][0]/255)*255,int(pix[x,y][0]/255)*255,255])
    #im.save('./BadApple-Frames/bad_apple_' + formatI(group*15+frame) + '.png')


for img in range(0, len(tmpValList)):
    for block in range(0, len(tmpValList[0])):
        val = (img*183+block) if (img*183+block)<=65535 else (img*183+block-65535)
        bnk = "2" if (img*183+block)<=65535 else "1"
        finalStr += "set " + str(val) + " " + str(tmpValList[img][block]) + " "+bnk+"\n"

print()




finalStr += '"'

with open("frame_data.arm", "w") as text_file:
    text_file.write(finalStr)

# print(finalStr)

print("Done Processing :: " + str((len(tmpValList)-1)*183+len(tmpValList[0])-1))