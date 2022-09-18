import csv
import json
from multiprocessing.dummy import Pool as ThreadPool
from threading import Lock

#Edit this script to set the paths and ids properly

#Mutex for threading
mutex = Lock()
pmutex = Lock()

#Paths
csvPath = "C:\\Users\\yoshi\\Downloads\\pokedex-master\\pokedex-master\\pokedex\\data\\csv\\pokemon_moves.csv"
jsonPath = "C:\\Users\\yoshi\\Downloads\\pokemon_moves.json"

#ID to make main key from
mainID = 'pokemon_id'

#ID to separate inner values from
innerID = 'move_id'

#Keep track of tasks
tasksTotal = 0

def processEntries(ids, data, path, tasksTotal):
    try:
        with open(path, encoding='utf-8') as csvf:
            csvReader = csv.DictReader(csvf)
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
        pmutex.acquire()
        print(ids, "done!")
        pmutex.release()
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
        tasks = 0
        for rows in csvReader:
            key = rows[mainID]
            tasks += 1
            if not key in pkmID:
                pkmID.append(key)
        tasksTotal = tasks * len(pkmID)
        print("Tasks: ", tasksTotal)
        print("Running...")
        
        #2.

        #Run thread for each id
        pool = ThreadPool(4)
        
        results = pool.starmap(processEntries, [(ids, data, path, tasksTotal) for ids in pkmID])

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
