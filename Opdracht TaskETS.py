import xlsxwriter

cpp_file = open("Bandgap.cpp", "r")
print(cpp_file)

# for line in cpp_file:
#     print(line)

row = 0
col = 0

list = []

def main():
    
    id = []
    testnr = []
    command = []
    lcl = []
    ucl = []
    unit = []
    rules = []
    dut_signal = []
    dut_pin = []
    tester_pin = []
    wait = []
    samples = []
    port = []
    measurement = []
    calculation = []
    comment = []
    x = []
    load_board_path = []
    device_state = []
    supply_state = []
    levels = []
    timing = []
    empty = []


workbook = xlsxwriter.Workbook('TaskETS.xlsx')
worksheet = workbook.add_worksheet('Industrial')


# test = ""
def findValue(search_for, curr_line, row, col):
    output_index = curr_line.find(search_for) # start of search_for
    if output_index == -1:
        print("NO")
        #exit()
    #print("OUTPUT: " + str(output_index))
    output_index2 = curr_line[output_index + len(search_for):].find('"') + output_index # find next ending
    # output_index3 = curr_line[output_index + len(search_for):].find(')') + output_index
    output_index3 = curr_line[output_index:].find(")") + len(search_for) + 1 #find
    output = curr_line[output_index + len(search_for):output_index2 + len(search_for)]
    #print(output_index, output_index2, output)
    #unecessary = curr_line[:output_index] + curr_line[output_index3:]
    #print("OUTPUT: " + output)
    #output_replace_index = (curr_line.find(output) - 1)
    #output_replace = curr_line[output_replace_index: (output_replace_index + (len(output) + 2))]


    unecessary = curr_line.replace(output, "")

    find_i = unecessary.find(search_for)
    find = unecessary[find_i: (find_i + (len(search_for) + 2))]

    unecessary = unecessary.replace(find, "")
    #print("haha", curr_line)
    #print("wtf?", unecessary)
    #print(output_index3)
    #print(curr_line[output_index:])
    worksheet.write(row, col, output)
    return unecessary
    #worksheet.write(row, col, testie)
    # return output_index, output_index2
    # end_line = curr_line.replace[output_index:output_index2]("")



# test = findValue('port("', curr_line, row, 13)
# print("hooioioi", test)

def findLine(file):
    end_line = ""
    row = 1
    col = 14

    for curr_line in file:
        if "rdi." in curr_line and 'port("' in curr_line:
            curr_line = curr_line.replace("rdi.", "")
            while curr_line[0] == "\t":
                curr_line = curr_line[1:]
            print("START: " + curr_line)
            # list = [curr_line]
            # test = list.replace("rdi.", "")
            # print(list)
            # list = [s.replace('rdi.', '') for s in list]
            # print(list)
            # test = [s.replace('rdi.', '') for s in curr_line]
            if ".cont();" in curr_line:
                # cont_index = curr_line.find(".cont()")
                # prin
                worksheet.write(row, 7, "=")
            # if "port" in curr_line:
            #     port_index = curr_line.find("port(")
            #     # port_index2 = curr_line[port_index:].find(")") + len(curr_line[:port_index])
            #     port_index2 = curr_line[port_index:].find(")") + port_index
            #     print(port_index)
            #     print(port_index2)
            #     print(curr_line)
            #     print(curr_line[port_index:])
            #     port = curr_line[port_index + len('port("'):port_index2 -1]
            #     print("hoi", port)
            #     worksheet.write(row, 13, port)
            '''
            test
            '''
            if ('port("') in curr_line:
                print("curr_line:", curr_line)
                index_test = curr_line.find('port("')
                index_test2 = curr_line[index_test + len('port(")')].find(')')
                index_test3 = curr_line[index_test].find(")") + index_test
                print("BISH", index_test, index_test3)


            if 'port.("@"' not in curr_line:
                if 'port("' in curr_line:
                    print("PORT")
                    end_line = findValue('port("', curr_line, row, 13)
            if '.pin("' in curr_line:
                end_line = findValue('.pin("', end_line, row, 9)
            if '.dc("' in curr_line:
                end_line = findValue('.dc("', end_line, row, 3)
            if '.func().label("' in curr_line:
                end_line = findValue('.func().label("', end_line, row, 3)

            worksheet.write(row, 14, end_line)






            # print(output_index)
            # end_line = curr_line.replace[output_index:output_index2]("")
            # end_line = curr_line[:output_index] + curr_line[output_index2:]
            # test = findValue('port("', curr_line, row, 13)
            # if test != None:
            #     a = min(test)
            #     b = max(test)
            #     print("omg", a)
            #     print(b)
            #     return a, b
            #
            # print(a)
            # if test != "NoneType":
            #     a,b = test.split(",")
            # a = min
            # print("HOOII", test)
            # print(a)
            # print(b)
            # print(a)
            # print(b)
            # end_line = curr_line[]

            if 'dshfadskuhflak' in curr_line:
                end_line = findValue("port", curr_line, row, col)
                end_line = findValue(".pin", end_line, row, col)
                end_line = findValue(".dc", end_line, row, col)
                end_line = end_line.replace("rdi.", "")
                worksheet.write(row, col, end_line)
                # end_line = findValue(".pin", end_line, row, col)
                # end_line = findValue(".dc", end_line, row, col)
                # return end_line
            # print("hooironefjriofer", end_line)
            # end_line = end_line.findValue(".pin", curr_line, row, col)
            # end_line = end_line.replace("rdi.", "")
            # end_line = end_line.replace(".execute();", "")
            # end_line = end_line.replace(".cont();", "")
            # end_line = end_line.replace("port", "")
            # end_line = end_line.replace('port("@"', "")
            # end_line = end_line.replace(".pin", "")
            # end_line = end_line.replace(".average", "")
            # end_line = end_line.replace(".dc", "")
            # end_line = end_line.replace(".func().label", "")
            # end_line = end_line.replace("", "")
            print("ENDLINE: ", end_line)
            print("END\n")
            # worksheet.write(row, col, end_line)
            row += 1

findLine(cpp_file)
# print(test)

# print(original)
# print([s.replace('M', '') for s in original])



'''
def findLine(file, to_find):
    if to_find == "rdi.":
        row = 1
        col = 14
    # elif to_find == "":
    #     row = 0
    #     col = 0

    for curr_line in file:
        if to_find in curr_line:
            worksheet.write(row, col, curr_line)
            row += 1
        if ".cont();" in curr_line:
            col = 7
            worksheet.write(row, col, "=")
            col = 14
        if ".port" in curr_line:
            col = 13
            worksheet.write(row, col, "port")
            col = 14
        if ".pin" in curr_line:
            col = 9
            worksheet.write(row, col, "pin")
            col = 14
        if ".average" in curr_line:
            col = 24
            worksheet.write(row, col, "average")
            col = 14
        if ".dc" in curr_line:
            col = 1
            worksheet.write(row, col, "dc")
            col = 14
        if ".func().label" in curr_line:
            col = 1
            worksheet.write(row, col, "func")
            col = 14
        if "//" or "/*" in curr_line:
            col = 16
            worksheet.write(row, col, "commentos")
            col = 14

            # print(curr_line)


findLine(cpp_file, "rdi.")
'''




'''
rows = (
    ["ID", "TestNR", "Command", ""],
    ["TestNR"],
    ["Command"],
    ["TestName"],
    ["LCL"],
    ["UCL"],
    ["[Unit]"],
    ["Rules"],
    ["DUT Signal"],
    ["DUT Pin"],
    ["Tester Pin"],
    ["Wait"],
    ["Samples"],
    ["Port"],
    ["Measurement"],
    ["Calculation"],
    ["Comment"],
    ["X"],
    ["Load Board path"],
    ["Device State"],
    ["Supply state"],
    ["Levels"],
    ["Timing"],
    [""]
)
'''



blue_row = workbook.add_format({"font_color": "white", "bg_color": "#4774c5"})
black_col = workbook.add_format({"font_color": "white", "bg_color": "black"})

worksheet.write(0, 0, "ID", blue_row)
worksheet.write(0, 1, "TestNR", blue_row)
worksheet.write(0, 2, "Command", blue_row)
worksheet.write(0, 3, "TestName", blue_row)
worksheet.write(0, 4, "LCL", blue_row)
worksheet.write(0, 5, "UCL", blue_row)
worksheet.write(0, 6, "[Unit]", blue_row)
worksheet.write(0, 7, "Rules", blue_row)
worksheet.write(0, 8, "DUT Signal", blue_row)
worksheet.write(0, 9, "DUT Pin", blue_row)
worksheet.write(0, 10, "Tester Pin", blue_row)
worksheet.write(0, 11, "Wait", blue_row)
worksheet.write(0, 12, "Samples", blue_row)
worksheet.write(0, 13, "Port", blue_row)
worksheet.write(0, 14, "Measurement", blue_row)
worksheet.write(0, 15, "Calculation", blue_row)
worksheet.write(0, 16, "Comment", blue_row)
worksheet.write(0, 17, "X", black_col)
worksheet.write(0, 18, "Load Board Path", blue_row)
worksheet.write(0, 19, "Device State", blue_row)
worksheet.write(0, 20, "Supply State", blue_row)
worksheet.write(0, 21, "Levels", blue_row)
worksheet.write(0, 22, "Timing", blue_row)
worksheet.write(0, 23, "", blue_row)
worksheet.write(0, 24, "Average", blue_row)





'''
for item, cost in (expenses):
    worksheet.write(row, col, item)
    worksheet.write(row, col + 1, cost)
    row += 1

worksheet.write(row, 0, 'Total')
worksheet.write(row, 1, '=SUM(B1:B4)')
'''

workbook.close()

# test = "blabla"
# test = test.replace("bl", "")[0:5]
# print(test)


# test = (5, 8)
# print(test)
# a = min(test)
# b = max(test)
# print(a)
# print(b)
