using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System;

public class GetJointAngles : MonoBehaviour {

    //public GameObject Right_ForeArm;
    //public GameObject Right_UpperArm;
    //public GameObject Right_Shoulder;
    //public GameObject Left_ForeArm;
    //public GameObject Left_UpperArm;
    //public GameObject Left_Shoulder;
    //public GameObject Chest;


    bool sdInserted = true;
    StreamWriter sdReader;
    

    const int num_joints = 7;
    public GameObject[] Joints = new GameObject[num_joints];

    Quaternion[] quat_Arr = new Quaternion[num_joints];

    // Use this for initialization
    void Start ()
    {
        try
        {
            string path = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
            new DirectoryInfo(path + "\\MoCap").Create();
            sdReader = new StreamWriter(path + "\\MoCap\\initial_orien.txt");
        }
        catch(DirectoryNotFoundException DNFE)
        {
            sdInserted = false;
            Debug.LogError(DNFE.Message);
        }
        //ConfirmLocal_WorldOrien();

        for (int i = 0; i < num_joints; i++)
        {
            quat_Arr[i] = Joints[i].transform.rotation;
            Debug.Log((char)(i + 65) + " : quat W: " + quat_Arr[i].w.ToString("0.##"));
            Debug.Log((char)(i + 65) + " : quat X: " + quat_Arr[i].x.ToString("0.##"));
            Debug.Log((char)(i + 65) + " : quat Y: " + quat_Arr[i].y.ToString("0.##"));
            Debug.Log((char)(i + 65) + " : quat Z: " + quat_Arr[i].z.ToString("0.##"));

            if(sdInserted)
            {
                sdReader.Write("{" + quat_Arr[i].w.ToString("0.###") + "," + quat_Arr[i].x.ToString("0.###") + "," + quat_Arr[i].y.ToString("0.###") + "," + quat_Arr[i].z.ToString("0.###") + "}");
                if (i < num_joints - 1)
                    sdReader.WriteLine(",");
            }
        }
        sdReader.Close();
        Debug.Log("SD transfer Completed!");

    }


    // Update is called once per frame
    void Update()
    {

    }
       
    void ConfirmLocal_WorldOrien()
    {
        Vector3 angles = Joints[6].transform.eulerAngles;
        Debug.Log("Angle X: " + angles.x);
        Debug.Log("Angle Y: " + angles.y);
        Debug.Log("Angle Z: " + angles.z);

        Vector3 localangles = Joints[6].transform.localEulerAngles;
        Debug.Log("LocalAngle X: " + localangles.x);
        Debug.Log("LocalAngle Y: " + localangles.y);
        Debug.Log("LocalAngle Z: " + localangles.z);

        Vector3 rotation = Joints[6].transform.rotation.eulerAngles;
        Debug.Log("rotation X: " + rotation.x);
        Debug.Log("rotation Y: " + rotation.y);
        Debug.Log("rotation Z: " + rotation.z);

        Vector3 localrotation = Joints[6].transform.localRotation.eulerAngles;
        Debug.Log("localrotation X: " + localrotation.x);
        Debug.Log("localrotation Y: " + localrotation.y);
        Debug.Log("localrotation Z: " + localrotation.z);

        Quaternion quat = Joints[6].transform.rotation;
        Debug.Log("quat W: " + quat.w);
        Debug.Log("quat X: " + quat.x);
        Debug.Log("quat Y: " + quat.y);
        Debug.Log("quat Z: " + quat.z);

        Quaternion localQuat = Joints[6].transform.localRotation;
        Debug.Log("localQuat W: " + localQuat.w);
        Debug.Log("localQuat X: " + localQuat.x);
        Debug.Log("localQuat Y: " + localQuat.y);
        Debug.Log("localQuat Z: " + localQuat.z);

        Vector3 dataAngles = new Vector3(0, 90, 180);

        Joints[6].transform.eulerAngles = dataAngles;

        Vector3 angles2 = Joints[6].transform.eulerAngles;
        Debug.Log("angles2 X: " + angles2.x);
        Debug.Log("angles2 Y: " + angles2.y);
        Debug.Log("angles2 Z: " + angles2.z);

        Vector3 localangles2 = Joints[6].transform.localEulerAngles;
        Debug.Log("localangles2 X: " + localangles2.x);
        Debug.Log("localangles2 Y: " + localangles2.y);
        Debug.Log("localangles2 Z: " + localangles2.z);

        Quaternion quat2 = Joints[6].transform.rotation;
        Debug.Log("quat2 W: " + quat2.w);
        Debug.Log("quat2 X: " + quat2.x);
        Debug.Log("quat2 Y: " + quat2.y);
        Debug.Log("quat2 Z: " + quat2.z);

        Quaternion localQuat2 = Joints[6].transform.localRotation;
        Debug.Log("localQuat2 W: " + localQuat2.w);
        Debug.Log("localQuat2 X: " + localQuat2.x);
        Debug.Log("localQuat2 Y: " + localQuat2.y);
        Debug.Log("localQuat2 Z: " + localQuat2.z);
    }

}
