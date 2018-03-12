# face_asm
This program demo was my coursework of the Digital Image Processing on Oct 2015, using the ASM model to detect vital feature points on human faces.
So the algorithm may be more traditional compared with the nn mainstream model.

## Recommended Environment
visual studio 2013 +opencv3.0
#Dataset 
muct-master dataset
you can download form [here](www.milbo.org/muct).

## Train Standard face templates
Images from muct-master were trained by the decision tree to get 30 standard face templates.

![](https://i.imgur.com/M4PtZTU.png)

you can use two sliders to adjust the parameter.

## Test

![](https://i.imgur.com/199r9UW.png)

I haven't computed evaluating indicators yet cause it's just my course practice.
If you are interested about this model,you can do some futher work.

## command 
test: -m [.model/model path] -f -C [face_alt.xml/face templates path] -p [.png/image path]  
you can see more parameter information from guideing help in vs cmd when you run the program. 
