using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using Firebase;
using Firebase.Database;
using Firebase.Unity.Editor;
using UTJ.FrameCapturer;


public class RunAlgorithms : MonoBehaviour
{
    List<List<SensorData>> data = new List<List<SensorData>>();
    IDictionary<string, object> iDictDT;

    Dictionary<string, object> timeStampDic = new Dictionary<string, object>();

    // Occurence Stuff
    Dictionary<string, Dictionary<string, object>> occDics = new Dictionary<string, Dictionary<string, object>>();
    Dictionary<string, object> totalOccDic = new Dictionary<string, object>();

    MovieRecorder mRec = new MovieRecorder();

    UInt32 cummDataCount = 0;
    double cummERMScore = 0.0;
    int i = 1;
    int k = 1; // Used to keep risk calculations in sync with the incoming data. I.e. error doesn't calculate twice for every one data pt

    const int num_joints = 7;
    public GameObject[] Joints = new GameObject[num_joints];
    Vector3[] jointPos = new Vector3[num_joints];

    ERM erm = new ERM();

    void Awake()
    {
       
    }

    // Use this for initialization
    void Start()
    {
        timeStampDic.Add("currentTime", ServerValue.Timestamp);

        #region Updates time on server and then reveiceves the value of time and prints it out (Testing)
        //Dictionary<string, object> dic = new Dictionary<string, object>();
        //dic.Add("currentTime", ServerValue.Timestamp);

        //Router.AssesmentWithID("-L5ohOlG020TA2K3tXrg").UpdateChildrenAsync(dic).ContinueWith(t =>
        //{
        //    if (t.IsFaulted)
        //    {
        //        // Handle the error...
        //        Debug.Log("Error Getting File (Once)!");
        //    }
        //    else if (t.IsCompleted)
        //    {
        //        #region GetValueAsync
        //        Router.AssesmentWithID("-L5ohOlG020TA2K3tXrg")//.Child("endDate")
        //          .GetValueAsync().ContinueWith(task =>
        //          {
        //              if (task.IsFaulted)
        //              {
        //                                // Handle the error...
        //                                Debug.Log("Error Getting File (Once)!");
        //              }
        //              else if (task.IsCompleted)
        //              {
        //                  DataSnapshot snapshot = task.Result;
        //                                // Do something with snapshot...
        //                                IDictionary<string, object> idict = (IDictionary<string, object>)snapshot.Value;
        //                  foreach (string s in idict.Keys)
        //                      Debug.Log("Key: " + s);

        //                  string str = idict["currentTime"].ToString();
        //                  Debug.Log("Snapshot: " + str);
        //                  DateTime dt = Milli2DateTime(str);
        //                  Debug.LogFormat("DateTime: {0}", dt.ToLocalTime().ToString());
        //                  Debug.Log(DateTime2Milli(dt));
        //              }
        //          });
        //        #endregion
        //    }

        //});
        #endregion

        #region Checks which Assessments are active and picks the employee ID of which ever one is.
        DatabaseManager.sharedInstance.GetActiveAssessments(result =>
        {
            Router.AID = result[0];
        });
        #endregion

        #region Pushes a set of dummy sensor Data to the server
        PushDummySenData();
        #endregion

        #region  Retrieves Sensor Data fron the Server
        //Router.DataWithEmpID(Router.EID).GetValueAsync().ContinueWith(t =>
        //{
        //    if (t.IsFaulted)
        //    {
        //        // Handle the error...
        //        Debug.Log("Error Getting File (Once)!");
        //    }
        //    else if (t.IsCompleted)
        //    {
        //        DataSnapshot dSnap = t.Result;
        //        IDictionary<string, object> iDictVal;

        //        List<List<List<SensorData>>> list1 = new List<List<List<SensorData>>>();
        //        List<List<object>> listIn = new List<List<object>>();
        //        List<List<SensorData>> list2 = new List<List<SensorData>>();
        //        List<SensorData> list3 = new List<SensorData>();

        //        iDictDT = (IDictionary<string, object>)dSnap.Value; // DateTimeStamp Level // IDictionary<string, object>
        //        //List<string> keyDT = new List<string>();
        //        foreach (string s in iDictDT.Keys)
        //        {
        //            // Gets the Key of the added snapshot converted to IDictionary. In this case, the key is the DateTimeStamp used to store sensorData on Firrebase.
        //            //keyDT.Add(s);
        //            Debug.Log("keyDT: " + s);
        //            listIn.Add((List<object>)iDictDT[s]); // DataCapture Level (#)

        //            int i = 0;
        //            foreach (List<object> L2 in listIn)// Loop through the DataCaptures (#)
        //            {
        //                Debug.Log("List Num: " + L2);
        //                int j = 0;
        //                foreach (object obj in L2)// Loop through each of the Sensors -> j
        //                {
        //                    iDictVal = (IDictionary<string, object>)obj;
        //                    //int k = 0;
        //                    foreach (IDictionary<string, object> iD_sData in iDictVal.Values) //Loop through each of the Quaternion Data -> k
        //                    {
        //                        Debug.Log("sData: " + iD_sData);
        //                        Debug.Log("J: " + j);
        //                        SensorData sense = new SensorData(iD_sData);
        //                        list3.Add(sense);
        //                        Debug.Log("Sense: " + sense.Qw);
        //                        //k++;
        //                    }
        //                    list2.Add(list3);
        //                    j++;
        //                }
        //                list1.Add(list2);
        //                i++;
        //            }
        //            float y;
        //            float.TryParse("0.256", out y);
        //            Debug.Log(y);
        //            Debug.Log("Added TempData");
        //            tempData.Add(list1);
        //        }
        //        Debug.Log("YEYEYEYEYEYE");
        //        Debug.Log("TempData: " + tempData[0][0][0][0].Qw);
        //    }

        //});
        #endregion

        #region Confirms whether increasing the millisecond value changes the DateTime value accurately
        //DateTime dt2 = DateTime.Now; // Get current Date&Time
        //Debug.Log("Current Time is: " + dt2);
        //string dd = DateTime2Milli(dt2); // Convert current Date&Time to millisecond (time since the Unix epoch)
        //Debug.Log("DD (Before adding 60000): " + dd);
        //UInt64 t = Convert.ToUInt64(dd); // Convert millisecond value from string to Uint64
        //Debug.Log("T : " + t);
        //t = t + 60000; // Increases millisecond value by 1 minute
        //Debug.Log("T (After adding 60000): " + t);
        //Debug.Log("New Time is: " + Milli2DateTime(t.ToString()));
        //IDictionary<string, object> idict2 = (IDictionary<string, object>)ServerValue.Timestamp;
        //foreach (string s in idict2.Keys)
        //{
        //    Debug.Log("Key2: " + s);
        //    Debug.Log("ServerValue: " + idict2[s].ToString());
        //}
        #endregion
    }


    // Update is called once per frame
    void Update()
    {
        //Debug.Log("i     " + i);
        //Debug.Log("k     " + k);
        //if (i >= DatabaseManager.Data.Count)
        //{
        //    //updateRiskOcc();
        //    Debug.Log("Updating!!!!");
        //}

        //bool[] risky = new bool[3];

        //if (k == i) // Used to keep risk calculations in sync with the incoming data. I.e. error doesn't calculate twice for every one data pt
        //{
        //    risky[0] = erm.checkBack((jointPos[2].z - jointPos[3].z) * 1000); // Compares the difference between waist location and neckish location and triggers risk when less than limit (Bending)
        //    risky[1] = erm.checkElbow_L((jointPos[0].z - jointPos[1].z) * 1000); // Compares the difference between Left elbow location and Left shou (waist) location and triggers risk when less than limit (Bending)
        //    risky[2] = erm.checkElbow_R((jointPos[6].z - jointPos[5].z) * 1000); // Compares the difference between shoulder location and back (waist) location and triggers risk when less than limit (Bending)

        //    if (risky[0] || risky[1] || risky[2])
        //    {
        //        Debug.Log("Risky 0 : " + risky[0]);
        //        Debug.Log("Risky 1 : " + risky[1]);
        //        Debug.Log("Risky 2 : " + risky[2]);
        //        cummERMScore += erm.ERM_Risk;Debug.Log("DatabaseManager.DTStamp: " + DatabaseManager.DTStamp);
        //        occDics[DatabaseManager.DTStamp].Add("ermScore", erm.ERM_Risk); 
        //        //occDics[DatabaseManager.DTStamp].Add("gif", );
        //    }
        //    k++;
        //}
    }

    void FixedUpdate()
    {
        //Debug.Log("i: " + i);
        //Debug.Log("k: " + k);
        //Debug.Log("Data.Count: " + DatabaseManager.Data.Count);
        //Debug.Log("Data[2].Count: " + DatabaseManager.Data[2].Count);
        Debug.Log("DatabaseManager.DataChanged: " + DatabaseManager.DataChanged);
        if (DatabaseManager.DataChanged == true) //(i >= DatabaseManager.Data.Count && DatabaseManager.DataChanged == true)
        {
            mRec.EndRecording();
            DatabaseManager.DataChanged = false;
            data = DatabaseManager.Data;
            i = 0;
            k = i;// Used to keep risk calculations in sync with the incoming data. I.e. error doesn't calculate twice for every one data pt
            cummDataCount = (uint)data.Count;

            mRec.BeginRecording();

            //Debug.Log("Time Stamps: " + DatabaseManager.DTStamp);
            //List<List<SensorData>> Quat = DatabaseManager.Data;
            //Debug.Log("Quat: " + Quat[0][0].Qw); //[0][0][0][0].Qw
        }
        if(i == data.Count)
        {
            updateRiskOcc();
            Debug.Log("Updating!!!!");
        }

        if (data.Count != 0 && i < data.Count)
        {
            Joints[0].transform.rotation = SensData2Quat(data[i][0]);
            Joints[1].transform.rotation = SensData2Quat(data[i][1]);
            Joints[2].transform.rotation = SensData2Quat(data[i][2]);
            Joints[3].transform.rotation = SensData2Quat(data[i][3]);
            Joints[4].transform.rotation = SensData2Quat(data[i][4]);
            Joints[5].transform.rotation = SensData2Quat(data[i][5]);
            Joints[6].transform.rotation = SensData2Quat(data[i][6]);
            // Debug.Log("SensData2Quat(data[i][5]): " + SensData2Quat(data[i][5]));
            i++;

            for (int z = 0; z < num_joints; z++)
            {
                jointPos[z] = Joints[z].transform.position;
            }

            if (i  == k + 1)
            {
                bool[] risky = new bool[3];
                risky[0] = erm.checkBack((jointPos[2].z - jointPos[3].z) * 1000); // Compares the difference between waist location and neckish location and triggers risk when less than limit (Bending)
                risky[1] = erm.checkElbow_L((jointPos[0].z - jointPos[1].z) * 1000); // Compares the difference between Left elbow location and Left shou (waist) location and triggers risk when less than limit (Bending)
                risky[2] = erm.checkElbow_R((jointPos[6].z - jointPos[5].z) * 1000); // Compares the difference between shoulder location and back (waist) location and triggers risk when less than limit (Bending)
                
                if (risky[0] || risky[1] || risky[2])
                {
                    //Debug.Log("Risky 0 : " + risky[0]);
                    //Debug.Log("Risky 1 : " + risky[1]);
                    //Debug.Log("Risky 2 : " + risky[2]);
                    cummERMScore += erm.ERM_Risk; //Debug.Log("DatabaseManager.DTStamp: " + DatabaseManager.DTStamp);
                    //Debug.Log("Im in");
                    Dictionary<string, object> dit = new Dictionary<string, object>();
                    dit.Add("ermScore", erm.ERM_Risk);
                    string x = ((Convert.ToInt64(DatabaseManager.DTStamp)) + (i * 50)).ToString(); //Debug.Log("DatabaseManager.DTStamp: " + x);
                    try
                    {
                        occDics.Add(x, dit); // "+ (i*50)" is used to determine the approx time of the sub-captures assuming 50ms per sub-capture

                    }catch(ArgumentException AE)
                    {
                        Debug.LogError("key: " + x + " : " + AE.Message);
                    }
                    //occDics[DatabaseManager.DTStamp].Add("gif", );
                }
                k = i;
            }
            
        }
    }

    DateTime Milli2DateTime(string milli)
    {
        return new DateTime(1970, 1, 1).AddMilliseconds(double.Parse(milli) /** 1000*/);
    }

    string DateTime2Milli(DateTime dt)
    {
        DateTime firstDt = new DateTime(1970, 1, 1);
        return (Math.Truncate(dt.Subtract(firstDt).TotalMilliseconds) /*/ 1000*/).ToString();
    }

    void PushDummySenData()
    {
        Dictionary<string, object> dic = new Dictionary<string, object>();
        dic.Add("currentTime", ServerValue.Timestamp);

        Router.AssesmentWithID(Router.AID).UpdateChildrenAsync(dic).ContinueWith(t =>
        {
            if (t.IsFaulted)
            {
                // Handle the error...
                Debug.Log("Error Getting File (Once)!");
            }
            else if (t.IsCompleted)
            {
                #region GetValueAsync
                Router.AssesmentWithID(Router.AID)//.Child("endDate")
                  .GetValueAsync().ContinueWith(task =>
                  {
                      if (task.IsFaulted)
                      {
                          // Handle the error...
                          Debug.Log("Error Getting File (Once)!");
                      }
                      else if (task.IsCompleted)
                      {
                          DataSnapshot snapshot = task.Result;

                          // Do something with snapshot...
                          IDictionary<string, object> idict = (IDictionary<string, object>)snapshot.Value;
                          //foreach (string s in idict.Keys)
                          //    Debug.Log("Key: " + s);

                          string str = idict["currentTime"].ToString();

                          Dictionary<string, object> dic2 = new Dictionary<string, object>();
                          dic2.Add("qw", 0.5); dic2.Add("qx", 0.5); dic2.Add("qy", 0.5); dic2.Add("qz", 0.5);

                          for (int j = 0; j < 10; j++)
                          {
                              for (int i = 0; i < 7; i++)
                              {
                                  Router.DataWithEmpID(Router.AID).Child(str).Child(j.ToString()).Child(((char)(i + 65)).ToString()).UpdateChildrenAsync(dic2);
                              }
                          }

                      }
                  });
                #endregion  
            }
        });
    }

    void updateRiskOcc()
    {
        #region Uploads the entries that goes into the occurences node on firebase
        foreach (string key in occDics.Keys)
        {
            foreach (Dictionary<string, object> occDic in occDics.Values)
                Router.OccurenceWithID(Router.AID).Child(key).UpdateChildrenAsync(occDic).ContinueWith(task =>
                {
                    occDics.Clear();
                });
        }
        #endregion


        #region Updates the entries associated with the risk in the Assessment node on firebase
        Router.AssesmentWithID(Router.AID)
          .GetValueAsync().ContinueWith(task =>
          {
              if (task.IsFaulted)
              {
                  // Handle the error...
                  Debug.Log("Error Getting File (Once)!");
              }
              else if (task.IsCompleted)
              {
                  DataSnapshot snapshot = task.Result;

                  // Do something with snapshot...
                  IDictionary<string, object> idict = (IDictionary<string, object>)snapshot.Value;

                  totalOccDic.Add("totalDataCount", (Convert.ToUInt32(idict["totalDataCount"]) + cummDataCount));
                  totalOccDic.Add("totalErmScore", (Convert.ToUInt32(idict["totalErmScore"]) + cummERMScore));

                  Router.AssesmentWithID(Router.AID).UpdateChildrenAsync(totalOccDic).ContinueWith(tk =>
                  {
                      totalOccDic.Clear();
                      cummDataCount = 0;
                      cummERMScore = 0;
                  });

              }
          });
        #endregion
    }

    Quaternion SensData2Quat(SensorData sens)
    {
        return new Quaternion(sens.Qx, sens.Qy, sens.Qz, sens.Qw);
    }

}
