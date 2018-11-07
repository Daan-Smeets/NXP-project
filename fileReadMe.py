import xlsxwriter

def findLine(input_file_name, to_find):
    file = open(input_file_name, 'r')

    total_found_lines = 0

    command_list = []
    value_list = []
    sweep_list = []
    description_list = []

    for curr_line in file:
        if to_find in curr_line:
            if ".i2cMap." in curr_line: #TODO: FIX to_find AND REMOVE .i2cMap
                total_found_lines += 1
                sweep_list.append(False)

                toFindIndex = curr_line.find((to_find))
                output_line = curr_line[toFindIndex:]
                output_line = output_line.replace(to_find, "")

                if "//" in output_line:
                    comment_index = output_line.find("//")
                    description = output_line[comment_index + 3:]
                    description_list.append(description)
                    output_line = output_line[:comment_index] + "\n"
                else:
                    description_list.append("")

                if "=" in output_line:  # Ask why there are lines without '=' (i2cMap.man_state.markForExpect ( 0x9 );)
                    equal_index = output_line.find("=")
                    semicolon_index = output_line.find(";")
                    command = output_line[:equal_index - 1]
                    value = output_line[equal_index + 2: semicolon_index]
                    command_list.append(command)
                    value_list.append(value)
                else:
                    total_found_lines -= 1
                print(output_line, end="")
            elif ".write" in curr_line:
                sweep_list.append(True)

    excel("testStatesMy.xlsx", total_found_lines, command_list, value_list, sweep_list, description_list)

    file.close()

def excel(file_name, total_found_lines, command_list, value_list, sweep_list, description_list):
    row = 0
    col = 0

    workbook = xlsxwriter.Workbook(file_name)
    test_state_labels = workbook.add_worksheet("test_state_labels")

    patroon_1 = workbook.add_worksheet("Patroon_1")

    blue_row = workbook.add_format({"font_color": "white", "bg_color": "#4774c5"})

    test_state_labels.write(0, 0, "Label Name", blue_row)
    test_state_labels.write(0, 1, "Protocol", blue_row)
    test_state_labels.write(0, 2, "Settings" ,blue_row)
    test_state_labels.write(0, 3, "Concurrent", blue_row)

    test_state_labels.write(1, 0, "Patroon_1")
    test_state_labels.write(1, 1, "I2C")


    patroon_1.write(0, 0, "Command", blue_row)
    patroon_1.write(0, 1, "Value", blue_row)
    patroon_1.write(0, 2, "Expected", blue_row)
    patroon_1.write(0, 3, "Sweep", blue_row)
    patroon_1.write(0, 4, "Description", blue_row)

    for i in range(0, total_found_lines):  # patroon_1.write(, , )
        col = 0
        row += 1

        patroon_1.write(row, col, command_list[i])
        col += 1
        patroon_1.write(row, col, value_list[i])
        col += 2
        if sweep_list[i] == True:
            patroon_1.write(row, col, "FLUSH")
        col += 1
        patroon_1.write(row, col, description_list[i])
        col += 1

    workbook.close()

findLine("Bandgap.cpp", "registers.")


