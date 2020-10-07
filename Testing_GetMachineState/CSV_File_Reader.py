import csv
import pandas as pd
def readcsvfile():
    data = pd.read_csv("Tumbler_1_230920.csv")
    data['IL']
    currents = data.IL
    print(currents)
    #with open('Tumbler_1_230920.csv') as file:
     #   reader = csv.reader(file)
     #   for row in reader:
     #       data = list(row[10])
     #       print(data)
    pass


