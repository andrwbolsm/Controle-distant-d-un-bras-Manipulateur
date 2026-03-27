tau = [0.091196 0.070928000000000 ...
       0.080997000000000 0.080872000000000 ...
       0.070855000000000 0.080995000000000
      ];

G = [10.966314312031230 14.098212271599370 ...
    12.346531353013075 12.364699772479968 ...
    14.112963093641945 12.346663374282365
    ];

n = 6; % joints

Kd = 1 ./ tau;

Trc_values = linspace(0.02, 1.0, 10);   % entre 20ms et 1s
N = length(Trc_values);

Kc_all = zeros(N, n); 

KGmin = 0.1;
KGmax = 1000;

for k = 1:N
    Trc = Trc_values(k);

    for i = 1:n
        f = @(KG) equation(KG, Kd(i), 0, Trc);
        KG_sol = fzero(f, [KGmin, KGmax]);

        Kc_all(k, i) = KG_sol / G(i);
    end
end

disp("Tau :");
disp(tau);

disp("Trc values :");
disp(Trc_values);
disp("Kc_all (chaque ligne = Kc pour un Trc) :");
disp(Kc_all);


% =======================
% Export CSV pour le C
% =======================
T = array2table([Trc_values(:), Kc_all], ...
    'VariableNames', ["Trc","Kc0","Kc1","Kc2","Kc3","Kc4","Kc5"]);

writetable(T, "Kc_LUT.csv");

disp("Lookup Table Kc_LUT.csv générée !");