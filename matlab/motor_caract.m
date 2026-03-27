data = readtable('latence.csv');

t = (data.t_rcv - data.t_rcv(1)) * 1e-6; 

n = 6; % joints

tau  = zeros(n,1);
K    = zeros(n,1);
Kd   = zeros(n,1);
G    = zeros(n,1);

for i = 1:n
    q_in  = data.(sprintf("q%d_in", i-1));
    q_rcv = data.(sprintf("q%d_rcv", i-1));

    u = q_in(end);
    y = q_rcv;

    % 63,2%
    y_inf = y(end);
    level = 0.632 * y_inf;

    [~, idx] = min(abs(y - level));
    tau(i) = t(idx);

    K(i) = y_inf / u;
    Kd(i) = 1 / tau(i);
    G(i)  = K(i) / tau(i);
end
