using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LimitChecker {

    /// <summary>
    /// Checks the value of the input data, and compares it to the limit.
    /// </summary>
    /// <param name="dataValue"> Value that changes that need to be checked. </param>
    /// <param name="limit"> The reference value being compared to. </param>
    /// <returns> Returns True if the data value is greateer than the limit provided. </returns>
    public bool Chk_Greater(float dataValue, float limit)
    {
        bool passedLimit;

        if (dataValue > limit)
        {
            passedLimit = true;
        }
        else { passedLimit = false; }

        return passedLimit;
    }

    /// <summary>
    /// Checks the value of the input data, and compares it to the limit.
    /// </summary>
    /// <param name="dataValue"> Value that changes and needs to be checked. </param>
    /// <param name="limit"> The reference value being compared to. </param>
    /// <returns> Returns True if the data value is less than the limit provided. </returns>
    public bool Chk_Lesser(float dataValue, float limit)
    {
        bool passedLimit;

        if (dataValue < limit)
        {
            passedLimit = true;
        }
        else { passedLimit = false; }

        return passedLimit;
    }

}
