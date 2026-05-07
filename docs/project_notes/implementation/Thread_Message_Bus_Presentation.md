# Thread Message-Bus Presentation Diagram

This is a simplified, PowerPoint-friendly runtime-only version of [[Thread_Message_Bus_Diagram]].

Use the SVG asset for slides:

![[thread_message_bus_presentation.svg]]

Use the editable PowerPoint version when you want to move boxes, edit labels, or
re-route arrows in PowerPoint:

[[Thread_Message_Bus_Presentation_Editable.pptx]]

## Simplification Notes

- The diagram is 16:9 and vector-based, so it can be inserted into PowerPoint without becoming blurry.
- It omits unused/debug-only paths so the slide focuses on the active runtime dataflow.
- It groups detailed channels into presentation-level labels where useful:
  - `haptics.wrenches` is shown as the object wrench queue.
  - `device.wrench_cmd` is shown as the device force queue.
  - `logging.device_timing`, `logging.device_state`, and `logging.sim_validation` are shown together as "Logging channels".
- The full named-channel list and payload details remain in [[Thread_Message_Bus_Diagram]].
