SD5 A6
    Stephen Merendino
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
[Completed Features]
    - All (but no property blocks for B3)

[Notes]
    - Press F3 to toggle color warp mode
    - ComputeShaders get added to database in Compute Shader constructor
    - I recommend using the arrow keys to start zooming and then switching to mouse wheel once you are zoomed in pretty good on a spot you like

[To Run From Source]
    - Compile in x86 (We were never able to compile in x64 last mod with Squirrel)
    - Set working directory of project to be $(SolutionDir)Run_$(PlatformName) in Visual Studio (otherwise the the app won't be able to read in files)
    - You can't pan the view when you have the sliders open

[Keyboard Controls]
    F1                                                                  - Toggle mouse visibility
    F2                                                                  - Open slider view
    Left/Right Arrow                                                    - Change the gradient
    Up/Down Arrow                                                       - Change Zoom
    Mouse Hold and Drag                                                 - Pan the camera
    Mouse Wheel                                                         - Zoom in/Zoom out

[Console]
    ~ (Tilde)                                                           - Open/Close Console

    A5
        None

    A4
        garbage_data_test_job [string:filename uint:byte_count]         - Runs a job that creates a file filled with garbage data
        log_copy [string:copy_filename]                                 - Copies the log to a new file, then opens the original log file in append mode
        load_cube_texture_async [string:texture_filename]               - Pauses all threads being profiled
        profile_job_system                                              - Runs my simple test to profile the public api of the job system

    A3
        profiler_pause_all                                              - Pauses all threads being profiled
        profiler_resume_all                                             - Resumes all threads being profiled
        profiler_step_all                                               - Steps all threads being profiled
        profiler_flat_report_all                                        - Prints out flat report of all threads to log
        profiler_tree_report_all                                        - Prints out tree report of all threads to log

    A2
        log [string:message]                                            - Log a message
        log_tagged [string:tag]                                         - Log a message with a tag
        log_callstack [string:message]                                  - Logs a message with its callstack
        log_tagged_callstack [string:tag string:message]                - Log a message with a tag and its callstack
        log_disable_all_tags                                            - Disable all tags
        log_enable_all_tags                                             - Enable all tags
        log_disable_tag [string:tag]                                    - Disable specific tag
        log_enable_tag [string:tag]                                     - Enable specific tag
        log_flush_test [string:message]                                 - Log a message, flush to file, then break
        log_thread_test [uint:thread_count uint:num_lines]              - Run multiple threads writing out multiple lines to file
        garbage_data_test_thread [string:filename uint:byte_count]      - Create a file and fill it with garbage

    A1
        memory_profiler                                                 - Print the current memory profile out to the console
        show_profiler                                                   - Shows the profiler
        hide_profiler                                                   - Hides the profiler
        log_live_memory_to_console [uint:start_frame uint:end_frame]    - Prints live allocations to console and visual studio debugger
        log_live_memory_to_file [string:filename]                       - Output live allocations to file
        leak_memory [uint:size]                                         - Leak memory
        set_particle_life_scale [float:scale]                           - Scale the life of particles

    General
        clear                                                           - Clears the developer console
        exit                                                            - Exits the developer console
        help                                                            - Shows all registered console commands
        list_config                                                     - Shows all settings that are currently loaded in the config system
        quit                                                            - Quits the app
        save_console [string:filepath]                                  - Saves the console log out to a file. Can't use spaces or quotes (eg: save_console console.txt)

    Controls
        Left/Right Arrow                                                - Move cursor in the input buffer
        Up/Down Arrow                                                   - Cycle through previously entered commmands
        Ctrl-C/Ctrl-X/Ctrl-V                                            - Copy/Cut/Paste like you would expect (works with windows clipboard)
        Page Up/Page Down                                               - Scroll the console log up and down
        Home/End                                                        - Jump to beginning/end of input buffer
        Shift                                                           - Hold down to highlight text in the input region (works with arrow keys and also home/end)
