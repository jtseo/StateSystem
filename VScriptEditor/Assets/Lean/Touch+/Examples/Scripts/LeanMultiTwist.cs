using UnityEngine;
using UnityEngine.Events;

namespace Lean.Touch
{
	/// <summary>This component allows you to get the twist of all fingers.</summary>
	[HelpURL(LeanTouch.PlusHelpUrlPrefix + "LeanMultiTwist")]
	[AddComponentMenu(LeanTouch.ComponentPathPrefix + "Multi Twist")]
	public class LeanMultiTwist : MonoBehaviour
	{
		// Event signature
		[System.Serializable] public class FloatEvent : UnityEvent<float> {}

		/// <summary>The method used to find fingers to use with this component. See LeanFingerFilter documentation for more information.</summary>
		public LeanFingerFilter Use = new LeanFingerFilter(true);

		/// <summary>If there is no twisting, ignore it?</summary>
		[Tooltip("If there is no twisting, ignore it?")]
		public bool IgnoreIfStatic;

		public FloatEvent OnTwistDegrees { get { if (onTwistDegrees == null) onTwistDegrees = new FloatEvent(); return onTwistDegrees; } } [UnityEngine.Serialization.FormerlySerializedAs("onTwist")] [SerializeField] private FloatEvent onTwistDegrees;

		/// <summary>If you've set Use to ManuallyAddedFingers, then you can call this method to manually add a finger.</summary>
		public void AddFinger(LeanFinger finger)
		{
			Use.AddFinger(finger);
		}

		/// <summary>If you've set Use to ManuallyAddedFingers, then you can call this method to manually remove a finger.</summary>
		public void RemoveFinger(LeanFinger finger)
		{
			Use.RemoveFinger(finger);
		}

		/// <summary>If you've set Use to ManuallyAddedFingers, then you can call this method to manually remove all fingers.</summary>
		public void RemoveAllFingers()
		{
			Use.RemoveAllFingers();
		}
#if UNITY_EDITOR
		protected virtual void Reset()
		{
			Use.UpdateRequiredSelectable(gameObject);
		}
#endif
		protected virtual void Awake()
		{
			Use.UpdateRequiredSelectable(gameObject);
		}

		protected virtual void Update()
		{
			// Get fingers
			var fingers = Use.GetFingers();

			if (fingers.Count > 0)
			{
				// Get twist
				var degrees = LeanGesture.GetTwistDegrees(fingers);

				// Ignore?
				if (IgnoreIfStatic == true && degrees == 0.0f)
				{
					return;
				}

				// Call events
				if (onTwistDegrees != null)
				{
					onTwistDegrees.Invoke(degrees);
				}
			}
		}
	}
}