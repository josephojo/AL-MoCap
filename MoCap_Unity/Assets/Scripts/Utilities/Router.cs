﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Firebase.Database;
using Firebase.Unity.Editor;

public class Router : MonoBehaviour {

    private static DatabaseReference baseRef = FirebaseDatabase.DefaultInstance.RootReference;
    //private static DatabaseReference dataDateRef = FirebaseDatabase.DefaultInstance.GetReference("users/")  

    private static string _eid = "-L6Iiv817U7M3HsjdMlH";
    private static string _aid = "-L5ohOlG020TA2K3tXrg";


    public static DatabaseReference Users()
    {
        return baseRef.Child("users");
    }

    public static DatabaseReference MainUserWithID()
    {
        return Users().Child("Ar07J0EG9hWlUwQvTBEeH0pvMXu2");
    }

    public static DatabaseReference UserWithID(string uid)
    {
        return Users().Child(uid);
    }

    public static DatabaseReference MainEmpWithID(string eid)
    {
        return MainUserWithID().Child("employees").Child(eid); // eid = "-L6Iiv817U7M3HsjdMlH"
    }

    public static DatabaseReference DataWithAssID(string aid)
    {
        return MainUserWithID().Child("data").Child(aid);
    }

    public static DatabaseReference Data()
    {
        return MainUserWithID().Child("data");
    }

    /// <summary>
    /// Provides a reference to the assessment node on Firebase 
    /// </summary>
    /// <param name="aid"> Asssessment ID</param>
    /// <returns>Returns a reference to the assesment entries with the given ID</returns>
    public static DatabaseReference AssesmentWithID(string aid)
    {
        return MainUserWithID().Child("assignments").Child(aid); // Called assignments instead of assessment in firebase database
    }
    public static DatabaseReference Assesment()
    {
        return MainUserWithID().Child("assignments"); // Called assignments instead of assessment in firebase database
    }

    /// <summary>
    /// Provides a reference to the Occurence node on Firebase
    /// </summary>
    /// <param name="aid"> Assessment ID</param>
    /// <returns> Returns a reference to the Occurence node on Firebase </returns>
    public static DatabaseReference OccurenceWithID(string aid)
    {
        return MainUserWithID().Child("occurences").Child(aid); // Called assignments instead of assessment in firebase database
    }

    /// <summary>
    /// Provides a reference to the Occurence node on Firebase
    /// </summary>
    /// <returns> Returns a reference to the Occurence node on Firebase </returns>
    public static DatabaseReference Occurence()
    {
        return MainUserWithID().Child("occurences"); // Called assignments instead of assessment in firebase database
    }

    public static string EID
    {
        set { _eid = value; }
        get { return _eid; }
    }

    public static string AID
    {
        set { _aid = value; }
        get { return _aid; }
    }

}
