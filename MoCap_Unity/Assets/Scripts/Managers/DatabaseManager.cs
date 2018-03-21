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
    static List<List<SensorData>> tempData = new List<List<SensorData>>();
    static string dtStamp = "A";
    static string prev_dtStamp;
    static bool dataChanged;

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

        Router.DataWithEmpID(Router.EID).LimitToLast(1).ChildAdded += HandleChildAdded;

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
            prev_dtStamp = dtStamp;
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
            //Debug.Log("dtStamp: " + dtStamp);
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

            //Debug.Log("Args.Snapshot.Key: " + args.Snapshot.Key); // Result = TimeStamp

            foreach (DataSnapshot dSnap in args.Snapshot.Children) // Loops through the DataCaptures (#s)
            {
                // Debug.Log("dSnap.Key: " + dSnap.Key); // Result = Data Captures (#)

                foreach (DataSnapshot dSnap1 in dSnap.Children) // Loops through the sensors (A-G)
                {
                    //Debug.Log("dSnap1.Key: " + dSnap1.Key); // Result = Sensor Letter (A,B,C etc)
                    iDictData = (IDictionary<string, object>)dSnap1.Value;

                    //Debug.Log("iDictData[QW]: " + iDictData["qw"]);
                    SensorData sense = new SensorData(iDictData);
                    list1.Add(sense); // Letters/Data
                                      //Debug.Log("Sense: " + sense.Qw);
                }
                list2.Add(list1.GetRange(0, list1.Count)); // DataCaptures/Letters/Data
                list1.Clear();
            }
            tempData.AddRange(list2);
            //Debug.Log("TempData: " + tempData[0].Count);
            dataChanged = true;
        }
        else
        {
            h++;
        }
    }

    public static List<List<SensorData>> Data
    {
        get
        {
            List<List<SensorData>> res = new List<List<SensorData>>();
            res.AddRange(tempData.GetRange(0, tempData.Count));
            return res;
        }
    }

    public static string DTStamp
    {
        get {return dtStamp; }
    }

    public static string prevDTStamp
    {
        get { return prev_dtStamp; }
    }

    public static bool DataChanged
    {
        set { dataChanged = value; }
        get { return dataChanged; }
    }

    public static void GetActiveAnalysis()
    {

    }
    

    void OnApplicationQuit()
    {
        Router.DataWithEmpID(Router.EID).LimitToLast(1).ChildAdded -= HandleChildAdded;
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
}
