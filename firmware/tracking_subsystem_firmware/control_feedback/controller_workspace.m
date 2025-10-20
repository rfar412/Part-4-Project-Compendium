%% PI Azimuth Control — Workspace Init
% Run this before opening/running the Simulink model.

%% Sampling & limits
Ts        = 0.25;      % [s] controller sample time (matches motor settle per 15° move)
AZ_MIN    = 0;         % [deg]
AZ_MAX    = 270;       % [deg]
AZ_TICK   = 15;        % [deg] command quantization step (one “tick”)

%% PI (discrete) with back-calculation anti-windup
Kp        = 4.47;      % proportional gain
Ki        = 0.167;      % integral gain [1/s]
Kb        = 1.00;      % back-calculation gain [1/s]
y         = 1; 
% If you use the Simulink PID (Discrete) block, configure:
%  - Form: PI
%  - Anti-windup method: Back-calculation
%  - Kb (tracking gain): use Kb above
%  - Output saturation: [AZ_MIN, AZ_MAX]
%  - Sample time: Ts

%% Quantizer helper (use in Simulink MATLAB Function or interpreted block)
quantizeToTick = @(u) AZ_TICK * round(u / AZ_TICK);

%% Reference trajectory for From Workspace block
%  - Use a step to 180° at t=0; change as needed.
t_end   = 30;                    % [s]
t_vec   = (0:Ts:t_end).';        % column vector
r_vec   = 180 * ones(size(t_vec));  % command (deg)

% Simulink "From Workspace" expects an Nx2 [time, value] array or struct
r_times  = t_vec;               
r_values = r_vec;              
r_ts     = [r_times, r_values]; 

%% Optional: simple “plant” parameters for SIL (if you simulate without hardware)
% A very simple angle hold plant: output jumps to commanded quantized angle
% in one sample. You can replace with your identified plant later.
plant_use_simple_hold = true;   
%% Serial link to Arduino (optional HIL)
% If you want Simulink to send "AZ <angle>" when u_q changes, open a port:
use_serial = false;   % set true when you want hardware in the loop         %#ok<NASGU>

if use_serial
    try
        % Adjust port name to your system, e.g., "COM7" (Windows) or "/dev/tty.usbmodemXXXX" (macOS)
        sp = serialport("COM7", 115200);  
        configureTerminator(sp, "LF");
        pause(0.2);
    catch ME
        warning("Serial init failed: %s", ME.message);
    end
end

%% Sender function for MATLAB Function block (Simulink)
% Use coder.extrinsic('evalin','writeline','sprintf') inside the block and call this protocol:
send_cmd = @(angle_deg) sprintf('AZ %d', round(angle_deg)); 

%% Sanity print
fprintf('[PI INIT] Ts=%.3fs  Kp=%.3f  Ki=%.3f  Kb=%.3f  Tick=%d°  Limits=[%d,%d]\n', ...
        Ts, Kp, Ki, Kb, AZ_TICK, AZ_MIN, AZ_MAX);


if ~exist('sp','var') || ~isvalid(sp)
    sp = serialport("COM7",115200); % <- your port
    configureTerminator(sp,"LF");
end

function uq = quantize_cmd(u, tick)
%#codegen
uq = tick * round(u / tick);
end

function send_if_changed(uq)
%#codegen
persistent last
if isempty(last); last = -1e9; end
if abs(uq - last) >= 15  % only on tick change
    coder.extrinsic('evalin','writeline','sprintf');
    sp = evalin('base','sp');              % serialport from workspace
    cmd = sprintf('AZ %d', round(uq));
    try
        writeline(sp, cmd);
    catch
        % no-op if serial not available
    end
    last = uq;
end
end
