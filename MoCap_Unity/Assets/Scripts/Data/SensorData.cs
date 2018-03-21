using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class SensorData {
    // Quaternion Values
    private float _qw;
    private float _qx;
    private float _qy;
    private float _qz;
    //private string _sensorName;

    public SensorData(IDictionary<string, object> iDict)
    {
        float.TryParse(iDict["qw"].ToString(), out _qw);
        float.TryParse(iDict["qx"].ToString(), out _qx);
        float.TryParse(iDict["qy"].ToString(), out _qy);
        float.TryParse(iDict["qz"].ToString(), out _qz);

        //foreach(string s in iDict.Keys)
        //{
        //    _sensorName = s;
        //}

    }

    public float Qw { get { return _qw; } }
    public float Qx { get { return _qx; } }
    public float Qy { get { return _qy; } }
    public float Qz { get { return _qz; } }
    //public string SensorName { get { return _sensorName; } }
}
