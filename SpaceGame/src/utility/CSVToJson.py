import csv
import json
from multiprocessing.dummy import Pool as ThreadPool
from threading import Lock

#Mutex for threading
mutex = Lock()

#Paths
csvPath = "C:\\Users\\yoshi\\Downloads\\pokedex-master\\pokedex-master\\pokedex\\data\\csv\\pokemon_moves.csv"
jsonPath = "C:\\Users\\yoshi\\Downloads\\pokemon_moves.json"

#ID to make main key from
mainID = 'pokemon_id'

#ID to separate inner values from
innerID = 'move_id'

def processEntries(ids, data, csvf, csvReader):
    try:
        csvf.seek(0)
        for rows in csvReader:
            key = rows[mainID]
            if key == ids:
                rows.pop(mainID)
                mutex.acquire()
                try:
                    if key not in data:
                        data[key] = dict()
                    data[key][rows[innerID]] = rows
                    data[key][rows[innerID]].pop(innerID)
                finally:
                    mutex.release()
        mutex.acquire()
        try:
            print("ID finished: ", ids)
        finally:
            mutex.release()
    except:
        print("Task has failed at: ", ids)

def csvRead(path):
    #Create data
    data = {}

    #Open CSV Reader
    with open(path, encoding='utf-8') as csvf:
        csvReader = csv.DictReader(csvf)

        #1. First run through and get a list of each unique pokemon id
        #2. For each id, run through, making a list of each row
        #3. At end of run through for a given id, add the list as the entry

        #1.
        pkmID = []
        for rows in csvReader:
            key = rows[mainID]
            if not key in pkmID:
                pkmID.append(key)

        #2.

        #Run thread for each id
        pool = ThreadPool(1)
        
        results = pool.starmap(processEntries, [(ids, data, csvf, csvReader) for ids in pkmID])
        pool.close()
            
    #Return data   
    return data

def jsonWrite(path, data):
    with open(jsonPath, 'w', encoding='utf-8') as jsonf:
        jsonf.write(json.dumps(data, indent=4))

def main():
    #Create data
    print("Reading data from declared path...")
    data = csvRead(csvPath)
    print("Writing data to path...")
    jsonWrite(jsonPath, data)
    print("Done!")

if __name__ == "__main__":
    main()
