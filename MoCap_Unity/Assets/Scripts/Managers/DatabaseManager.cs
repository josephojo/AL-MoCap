using System.Collections;
using System;
using System.Collections.Generic;
using UnityEngine;
using Firebase;
using Firebase.Database;
using Firebase.Unity.Editor;
using Firebase.Auth;

public class DatabaseManager : MonoBehaviour
{

    public static DatabaseManager sharedInstance = null;
    IDictionary<string, object> iDictDT;


    /* SensorData.List.List Structure is based on the following format
     *            |     
     *     Lists  V    SensorData List ->
     *     
     *     Number of   |   SensorData   |   SensorData   |   SensorData   |   SensorData   |   SensorData   |   SensorData   |   SensorData   |
     *     datacaptures|       0        |       1        |       2        |       3        |       4        |       5        |       6        |
     *     ------------|----------------|----------------|----------------|----------------|----------------|----------------|----------------|
     *           0     | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj |
     *           1     | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj |
     *           2     | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj |
     *           3     | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj | SensorData.Obj |  
     *           
     *     Nested List TempData:
     *         List of DataCaputures (Numbers)
     *                 List of Sensors in DataCapture
     *                                      
     */
    static List<SensorData> tempData = new List<SensorData>();  //List<List<SensorData>> tempData = new List<List<SensorData>>();
    static string dtStamp = "";
    static bool dataChanged;

    static Queue inputQ = new Queue();


    void Awake()
    {
        if (sharedInstance == null)
        {
            sharedInstance = this;
        }
        else if (sharedInstance != this)
        {
            Destroy(gameObject);
        }

        DontDestroyOnLoad(gameObject);
        FirebaseApp.DefaultInstance.SetEditorDatabaseUrl("https://ultima-apparel.firebaseio.com/");  //("https://ultima-apparel.firebaseio.com/"); // ("https://al-test-916f1.firebaseio.com/");
        tempData.Clear();

        //Router.DataWithAssID(Router.AID).ChildAdded += HandleChildAdded; //.LimitToLast(1)
        Router.DataWithAssID(Router.AID).ChildChanged += HandleChildChanged;
        //GetActiveAssessments(result =>
        //{
        //   // Debug.Log("Router.AID Count: " + result.Count);
        //    Router.AID = result[0];
        //    // Debug.Log("Router.AID: " + result[0]); // Router.AID);
        //});
        //randomTransmissions();

        //auth != null // For Firebase Read and write ruless

    }

    void Start()
    {

    }

    void Update()
    {

    }

    int h = 0;
    void HandleChildAdded(object sender, ChildChangedEventArgs args)
    {
        //Debug.Log("h: " + h);
        if (h > 0)
        {
            tempData.Clear();
            if (args.DatabaseError != null)
            {
                Debug.LogError(args.DatabaseError.Message);
                return;
            }
            // Do something with the data in args.Snapshot

            IDictionary<string, object> iDictData;
            List<List<SensorData>> list2 = new List<List<SensorData>>();
            List<SensorData> list1 = new List<SensorData>();

            //Debug.Log("Args.Snapshot.ChildrenCount: " + args.Snapshot.ChildrenCount);

            dtStamp = args.Snapshot.Key.ToString();
            Debug.Log("dtStamp: " + dtStamp);
            //Debug.Log("prevdtStamp: " + prev_dtStamp);

            #region Multiple Snapshots

            ////iDictDT = (IDictionary<string, object>) args.Snapshot.Value; // DateTimeStamp Level // IDictionary<string, object>
            ////foreach (string s in iDictDT.Keys)
            ////{
            ////    // Gets the Key of the added snapshot converted to IDictionary. In this case, the key is the DateTimeStamp used to store sensorData on Firrebase.
            ////    //keyDT.Add(s);
            ////    Debug.Log("keyDT: " + s);
            ////    listIn.Add((List<object>)iDictDT[s]); // DataCapture Level (#)
            //Debug.Log("Parent Key: " + args.Snapshot.Key);
            //Debug.Log("Counter: " + args.Snapshot.ChildrenCount);
            //foreach (DataSnapshot dSnap in args.Snapshot.Children)
            //{

            //    // Gets the Key of the added snapshot converted to IDictionary. In this case, the key is the DateTimeStamp used to store sensorData on Firrebase.
            //    //keyDT.Add(s);
            //    iDictDT = (IDictionary<string, object>)dSnap.Value;
            //    Debug.Log("iDictDT: " + iDictDT);

            //    foreach (string s in iDictDT.Keys)
            //    {
            //        Debug.Log("keyDT: " + s);
            //        //listIn.Add((List<object>)iDictDT[s]); // DataCapture Level (#)
            //    }
            //    //listIn.Add((List<object>)iDictDT[s]); // DataCapture Level (#)

            //    //int i = 0;
            //    //foreach (List<object> L2 in listIn)// Loop through the DataCaptures (#)
            //    //{
            //    //    Debug.Log("List Num: " + L2);
            //    //    int j = 0;
            //    //    foreach (object obj in L2)// Loop through each of the Sensors -> j
            //    //    {
            //    //        iDictData = (IDictionary<string, object>)obj;
            //    //        //int k = 0;
            //    //        foreach (IDictionary<string, object> iD_sData in iDictData.Values) //Loop through each of the Quaternion Data -> k
            //    //        {
            //    //            Debug.Log("sData: " + iD_sData); // iD_sData
            //    //            Debug.Log("J: " + j);
            //    //            SensorData sense = new SensorData(iD_sData);
            //    //            list1.Add(sense);
            //    //            Debug.Log("Sense: " + sense.Qw);
            //    //            //k++;
            //    //        }
            //    //        list2.Add(list1);
            //    //        j++;
            //    //    }
            //    //    list3.Add(list2);
            //    //    i++;
            //    //}
            //    //float y;
            //    //float.TryParse("0.256", out y);
            //    //Debug.Log(y);
            //    //Debug.Log("Added TempData");
            //    //tempData.Add(list3);

            //}
            #endregion

            Debug.Log("Args.Snapshot.Count: " + args.Snapshot.ChildrenCount); // Result = TimeStamp

            //foreach (DataSnapshot dSnap in args.Snapshot.Children) // Loops through the DataCaptures (#s)
            //{
            // Debug.Log("dSnap.Key: " + dSnap.Key); // Result = Data Captures (#)

            foreach (DataSnapshot dSnap1 in args.Snapshot.Children) //dSnap.Children) // Loops through the sensors (A-G)
            {
                Debug.Log("dSnap1.Key: " + dSnap1.Key); // Result = Sensor Letter (A,B,C etc)
                iDictData = (IDictionary<string, object>)dSnap1.Value;

                //Debug.Log("iDictData[QW]: " + iDictData["qw"]);
                SensorData sense = new SensorData(iDictData);
                list1.Add(sense); // Letters/Data
                                  //Debug.Log("Sense: " + sense.Qw);
            }
            list2.Add(list1.GetRange(0, list1.Count)); // DataCaptures/Letters/Data
            inputQ.Enqueue(list1.GetRange(0, list1.Count));
            //list1.Clear();
            //}
            tempData.AddRange(list1); //list2);
            list1.Clear();
            //Debug.Log("TempData: " + tempData[0].Count);
            dataChanged = true;
        }
        else
        {
            h++;
        }
    }

    void HandleChildChanged(object sender, ChildChangedEventArgs args)
    {
        //Debug.Log("h: " + h);
        if (h > 0)
        {
            tempData.Clear();
            if (args.DatabaseError != null)
            {
                Debug.LogError(args.DatabaseError.Message);
                return;
            }
            // Do something with the data in args.Snapshot

            IDictionary<string, object> iDictData;
            List<List<SensorData>> list2 = new List<List<SensorData>>();
            List<SensorData> list1 = new List<SensorData>();

            //Debug.Log("dtStamp: " + dtStamp);
            //Debug.Log("Args.Snapshot.ChildrenCount: " + args.Snapshot.ChildrenCount);

            #region Multiple Snapshots

            ////iDictDT = (IDictionary<string, object>) args.Snapshot.Value; // DateTimeStamp Level // IDictionary<string, object>
            ////foreach (string s in iDictDT.Keys)
            ////{
            ////    // Gets the Key of the added snapshot converted to IDictionary. In this case, the key is the DateTimeStamp used to store sensorData on Firrebase.
            ////    //keyDT.Add(s);
            ////    Debug.Log("keyDT: " + s);
            ////    listIn.Add((List<object>)iDictDT[s]); // DataCapture Level (#)
            //Debug.Log("Parent Key: " + args.Snapshot.Key);
            //Debug.Log("Counter: " + args.Snapshot.ChildrenCount);
            //foreach (DataSnapshot dSnap in args.Snapshot.Children)
            //{

            //    // Gets the Key of the added snapshot converted to IDictionary. In this case, the key is the DateTimeStamp used to store sensorData on Firrebase.
            //    //keyDT.Add(s);
            //    iDictDT = (IDictionary<string, object>)dSnap.Value;
            //    Debug.Log("iDictDT: " + iDictDT);

            //    foreach (string s in iDictDT.Keys)
            //    {
            //        Debug.Log("keyDT: " + s);
            //        //listIn.Add((List<object>)iDictDT[s]); // DataCapture Level (#)
            //    }
            //    //listIn.Add((List<object>)iDictDT[s]); // DataCapture Level (#)

            //    //int i = 0;
            //    //foreach (List<object> L2 in listIn)// Loop through the DataCaptures (#)
            //    //{
            //    //    Debug.Log("List Num: " + L2);
            //    //    int j = 0;
            //    //    foreach (object obj in L2)// Loop through each of the Sensors -> j
            //    //    {
            //    //        iDictData = (IDictionary<string, object>)obj;
            //    //        //int k = 0;
            //    //        foreach (IDictionary<string, object> iD_sData in iDictData.Values) //Loop through each of the Quaternion Data -> k
            //    //        {
            //    //            Debug.Log("sData: " + iD_sData); // iD_sData
            //    //            Debug.Log("J: " + j);
            //    //            SensorData sense = new SensorData(iD_sData);
            //    //            list1.Add(sense);
            //    //            Debug.Log("Sense: " + sense.Qw);
            //    //            //k++;
            //    //        }
            //    //        list2.Add(list1);
            //    //        j++;
            //    //    }
            //    //    list3.Add(list2);
            //    //    i++;
            //    //}
            //    //float y;
            //    //float.TryParse("0.256", out y);
            //    //Debug.Log(y);
            //    //Debug.Log("Added TempData");
            //    //tempData.Add(list3);

            //}
            #endregion

            //Debug.Log("Args.Snapshot.Count: " + args.Snapshot.ChildrenCount); // Result = TimeStamp
            if (args.Snapshot.ChildrenCount == 7)
            {
                dtStamp = args.Snapshot.Key.ToString();
                Debug.Log("dtStamp: " + dtStamp);
                //foreach (DataSnapshot dSnap in args.Snapshot.Children) // Loops through the DataCaptures (#s)
                //{
                // Debug.Log("dSnap.Key: " + dSnap.Key); // Result = Data Captures (#)

                foreach (DataSnapshot dSnap1 in args.Snapshot.Children) //dSnap.Children) // Loops through the sensors (A-G)
                {
                    //Debug.Log("dSnap1.Key: " + dSnap1.Key); // Result = Sensor Letter (A,B,C etc)
                    iDictData = (IDictionary<string, object>)dSnap1.Value;

                    //Debug.Log("iDictData[QW]: " + iDictData["qw"]);
                    SensorData sense = new SensorData(iDictData);
                    list1.Add(sense); // Letters/Data
                }
                list2.Add(list1.GetRange(0, list1.Count)); // DataCaptures/Letters/Data
                inputQ.Enqueue(list1.GetRange(0, list1.Count));

                tempData.AddRange(list1); //list2);
                list1.Clear();
                //Debug.Log("TempData: " + tempData[0].Count);
                dataChanged = true;
            }
        }
        else
        {
            h++;
        }
    }

    /// <summary>
    /// Property to return List that contains the List of each sensor(A, B, C etc) that sensorData (Quaternions - qw, qx, qy and qz) 
    /// </summary>
    public static List<SensorData> Data
    {
        get
        {
            List<SensorData> res = new List<SensorData>(); //List<List<SensorData>> res = new List<List<SensorData>>();
            res.AddRange(tempData.GetRange(0, tempData.Count));
            return res;
        }
    }

    public static UInt64 DTStamp
    {
        get
        {
            if (dtStamp == "")
                return 0;
            else
            return Convert.ToUInt64(dtStamp);
        }
    }

    public static bool DataChanged
    {
        set { dataChanged = value; }
        get { return dataChanged; }
    }

    public void GetActiveAssessments(Action<List<string>> completionBlock)
    {
        List<string> assessmentIDs = new List<string>();
        Router.Assesment().OrderByChild("active").EqualTo(true).GetValueAsync().ContinueWith(task =>
        {
            DataSnapshot activeAssessSnap = task.Result;

            foreach (DataSnapshot dSnap in activeAssessSnap.Children)
            {
                assessmentIDs.Add(dSnap.Key);
                //Debug.Log("Active Key: " + dSnap.Key);
                Router.AssesmentWithID(dSnap.Key).Child("employee").GetValueAsync().ContinueWith(task2 =>
                {
                    DataSnapshot empSnap = task2.Result;
                    Router.EID = empSnap.Value.ToString();
                    //Debug.Log("Emp ID: " + empSnap.Value.ToString());
                });
            }
            completionBlock(assessmentIDs);
        });
    }

    void OnApplicationQuit()
    {
        Router.DataWithAssID(Router.EID).LimitToLast(1).ChildAdded -= HandleChildAdded;
        Debug.Log("Application ending after " + Time.time + " seconds");
    }

    void randomTransmissions()
    {

        #region Uploads the info for Assesment Risk

        //Router.Assesment()//.Child("endDate")
        //          .GetValueAsync().ContinueWith(task =>
        //          {
        //              if (task.IsFaulted)
        //              {
        //                  // Handle the error...
        //                  Debug.Log("Error Getting File (Once)!");
        //              }
        //              else if (task.IsCompleted)
        //              {
        //                  List<string> s = new List<string>();
        //                  DataSnapshot snapshot = task.Result;

        //                  Dictionary<string, object> dic = new Dictionary<string, object>();
        //                  dic.Add("active", 0);
        //                  dic.Add("totalRiskscore", 0);
        //                  dic.Add("totalDataCount", 0);
        //                  dic.Add("totalErmScore", 0);

        //                  foreach (DataSnapshot snap in snapshot.Children)
        //                  {
        //                      Debug.Log(snap.Key);
        //                      Router.AssesmentWithID(snap.Key).UpdateChildrenAsync(dic);
        //                  }

        //              }
        //          });
        #endregion

        #region Updates the Timestamp
        //Dictionary<string, object> dic = new Dictionary<string, object>();
        //dic.Add("currentTime", ServerValue.Timestamp);

        //Router.Assesment().UpdateChildrenAsync(dic);

        #endregion


    }

    public static void DeleteAllData()
    {
        Router.DataWithAssID (Router.AID).RemoveValueAsync();
        Debug.Log("Deleting Data...");
    }
}
