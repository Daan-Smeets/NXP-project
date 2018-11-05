import xlwt


def findLine(inputFileName, toFind, outputFileName):
    file = open(inputFileName, 'r')
    txtFile = open(outputFileName, 'w')
    #print(file.read())
    lineIndex = 0
    i = 0

    for currLine in file:
        if toFind in currLine:
            fullLine = currLine
            toFindIndex = fullLine.find(toFind)
            outputLine = fullLine[toFindIndex:]
            outputLine = outputLine.replace(toFind, "")
            if "//" in outputLine:
                commentIndex = outputLine.find("//")
                outputLine = outputLine[:commentIndex] + "\n"# + "COMMENT "
                #print(commentIndex)
            #else:

            #print(outputLine + "Line: " + str(lineIndex))
            txtFile.write(outputLine)
            print(outputLine, end="")
            #print('Line: ' + str(lineIndex + 1))
        lineIndex+=1
    file.close()

def excel(filename, sheet, list1, list2, x, y, z):
    book = xlwt.Workbook()
    sh = book.add_sheet(sheet)

    variables = [x, y, z]
    x_desc = 'Display'
    y_desc = 'Dominance'
    z_desc = 'Test'
    desc = [x_desc, y_desc, z_desc]

    col1_name = 'Stimulus Time'
    col2_name = 'Reaction Time'

    #You may need to group the variables together
    #for n, (v_desc, v) in enumerate(zip(desc, variables)):
    for n, v_desc, v in enumerate(zip(desc, variables)):
        sh.write(n, 0, v_desc)
        sh.write(n, 1, v)

    n+=1

    sh.write(n, 0, col1_name)
    sh.write(n, 1, col2_name)

    for m, e1 in enumerate(list1, n+1):
        sh.write(m, 0, e1)

    for m, e2 in enumerate(list2, n+1):
        sh.write(m, 1, e2)

    book.save(filename)


findLine("Bandgap.cpp", "registers.i2cMap.", "textFile.txt")



# start()


