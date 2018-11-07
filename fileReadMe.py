import xlsxwriter

def findLine(inputFileName, toFind):
    file = open(inputFileName, 'r')

    totalFoundLines = 0

    commandList = []
    valueList = []
    sweepList = []
    descriptionList = []

    for currLine in file:
        if toFind in currLine:
            if ".i2cMap." in currLine:
                totalFoundLines += 1
                sweepList.append(False)
                fullLine = currLine
                toFindIndex = fullLine.find(toFind)
                outputLine = fullLine[toFindIndex:]
                outputLine = outputLine.replace(toFind, "")
                if "//" in outputLine:
                    commentIndex = outputLine.find("//")
                    description = outputLine[commentIndex + 3:]
                    descriptionList.append(description)
                    outputLine = outputLine[:commentIndex] + "\n"
                else:
                    descriptionList.append("")

                if "=" in outputLine:  # Ask why there are lines without '=' (i2cMap.man_state.markForExpect ( 0x9 );)
                    equalIndex = outputLine.find("=")
                    semicolonIndex = outputLine.find(";")
                    command = outputLine[:equalIndex - 1]
                    value = outputLine[equalIndex + 2: semicolonIndex]
                    commandList.append(command)
                    valueList.append(value)
                else:
                    totalFoundLines -= 1
                print(outputLine, end="")
            elif ".write" in currLine:
                sweepList.append(True)

    excel("testStatesMy.xlsx", totalFoundLines, commandList, valueList, sweepList, descriptionList)

    file.close()

def excel(filename, totalFoundLines, commandList, valueList, sweepList, descriptionList):
    row = 0
    col = 0

    workbook = xlsxwriter.Workbook(filename)
    worksheet = workbook.add_worksheet("TestStateLabels")
    patroon1 = workbook.add_worksheet("Patroon_1")

    blueRow = workbook.add_format({"font_color": "white", "bg_color": "#4774c5"})

    worksheet.write(0, 0, "Label Name", blueRow)
    worksheet.write(0, 1, "Protocol", blueRow)
    worksheet.write(0, 2, "Settings" ,blueRow)
    worksheet.write(0, 3, "Concurrent", blueRow)

    worksheet.write(1, 0, "Patroon_1")
    worksheet.write(1, 1, "I2C")


    patroon1.write(0, 0, "Command", blueRow)
    patroon1.write(0, 1, "Value", blueRow)
    patroon1.write(0, 2, "Expected", blueRow)
    patroon1.write(0, 3, "Sweep", blueRow)
    patroon1.write(0, 4, "Description", blueRow)

    for i in range(0, totalFoundLines):  # patroon1.write(, , )
        col = 0
        row += 1

        patroon1.write(row, col, commandList[i])
        col += 1
        patroon1.write(row, col, valueList[i])
        col += 1
        col += 1
        if sweepList[i] == True:
            patroon1.write(row, col, "FLUSH")
        col += 1
        patroon1.write(row, col, descriptionList[i])
        col += 1

    workbook.close()

findLine("Bandgap.cpp", "registers.")


