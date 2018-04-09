using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ERM {
    /// <summary>
    /// Constructor Enables the simultaneous limit definitions.
    /// </summary>
    /// <param name="backPose_Limit"> This is the limit value that triggers a bad bend pose - relative position of chest in the frontal and transverse axes. </param>
    /// <param name="elbowPose_L_Limit"> This is the limit value that triggers a bad elbow placement above the shoulder. </param>
    /// <param name="elbowPose_R_Limit"> This is the limit value that triggers a bad elbow placement above the Right shoulder </param>
    public ERM(float backPose_Limit, float elbowPose_L_Limit, float elbowPose_R_Limit)
    {
        _backPose_Limit = backPose_Limit;
        _elbowPose_L_Limit = elbowPose_L_Limit;
        _elbowPose_R_Limit = elbowPose_R_Limit;
    }


    public ERM()
    {
        // Limit in millimetres
        _backPose_Limit = 300.0f;
        _elbowPose_L_Limit = 0.0f;
        _elbowPose_R_Limit = 0.0f;
    }

    #region Fields

    private float _backPose_Weight = 0.4f;
    private float _elbowPose_L_Weight = 0.3f;
    private float _elbowPose_R_Weight = 0.3f;

    private float _backPose_Limit = 0.0f;
    private float _elbowPose_L_Limit = 0.0f;
    private float _elbowPose_R_Limit = 0.0f;

    private uint _back_Trig = 0;
    private uint _elbowL_Trig = 0;
    private uint _elbowR_Trig = 0;

    LimitChecker lim = new LimitChecker();

    #endregion

    #region Properties

    /// <summary>
    /// ERM Risk based on singular datastamp analyzed.
    /// </summary>
    public float ERM_Risk
    {
        get
        {
            return ((_elbowR_Trig * _elbowPose_R_Weight) + (_elbowL_Trig * _elbowPose_L_Weight) + (_back_Trig * _backPose_Weight));
        }
    }

    /// <summary>
    /// This is the limit value for triggering bad right elbow pose.
    /// </summary>
    public float RightElbowLimit
    {
        get
        {
            return _elbowPose_R_Limit;
        }

        set
        {
            _elbowPose_R_Limit = value;
        }
    }

    /// <summary>
    /// This is the limit value for triggering bad left elbow pose.
    /// </summary>
    public float LeftElbowLimit
    {
        get
        {
            return _elbowPose_R_Limit;
        }

        set
        {
            _elbowPose_R_Limit = value;
        }
    }

    /// <summary>
    /// This is the limit value for triggering bad back pose.
    /// </summary>
    public float BackLimit
    {
        get
        {
            return _elbowPose_R_Limit;
        }

        set
        {
            _elbowPose_R_Limit = value;
        }
    }


    /// <summary>
    /// This is the risk associated with the datastamp just analyzed of the right elbow pose.
    /// </summary>
    public float RightElbowRisk
    {
        get
        {
            return _elbowR_Trig * _elbowPose_R_Weight;
        }
    }

    /// <summary>
    /// This is the risk associated with the datastamp just analyzed of the right elbow pose.
    /// </summary>
    public float LeftElbowRisk
    {
        get
        {
            return _elbowL_Trig * _elbowPose_L_Weight;
        }
    }

    /// <summary>
    /// This is the risk associated with the datastamp just analyzed of the right elbow pose.
    /// </summary>
    public float BackRisk
    {
        get
        {
            return _back_Trig * _backPose_Weight;
        }
    }

    #endregion

    #region Methods

    /// <summary>
    /// Checks a value with the limit and confirms that it is greater than the limit.
    /// </summary>
    /// <param name="L_elbowPos">Data to be checked</param>
    /// <returns></returns>
    public bool checkElbow_L(float L_elbowPos)
    {
        bool result = lim.Chk_Greater(L_elbowPos, _elbowPose_L_Limit);

        if (result)
        {
            _elbowL_Trig = 1;
        }
        else
        {
            _elbowL_Trig = 0;
        }


        return result;
    }

    /// <summary>
    /// Checks a value with the limit and confirms that it is greater than the limit.
    /// </summary>
    /// <param name="R_elbowPos"> Data to be checked</param>
    /// <returns></returns>
    public bool checkElbow_R(float R_elbowPos)
    {
        bool result = lim.Chk_Greater(R_elbowPos, _elbowPose_R_Limit);

        if (result)
        {
            _elbowR_Trig = 1;
        }
        else
        {
            _elbowR_Trig = 0;
        }

        return result;
    }

    /// <summary>
    /// Checks a value with the limit and confirms that it is less than the limit.
    /// </summary>
    /// <param name="backPos"> Data to be checked</param>
    /// <returns></returns>
    public bool checkBack(float backPos)
    {
        bool result = lim.Chk_Lesser(backPos, _backPose_Limit);

        if (result)
        {
            _back_Trig = 1;
        }
        else
        {
            _back_Trig = 0;
        }

        return result;
    }

    #endregion

}
