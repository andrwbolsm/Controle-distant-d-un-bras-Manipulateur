function val = equation(KG, Kd, Kp, Trc)

    % discriminant
    delta = (Kd^4)/(KG^2) - 4*(Kp*(Kd^2)/(KG^2) - 1);

    if delta <= 0
        val = 1e6;   % pas de solution réelle
        return
    end

    % cos(theta)
    arg = (-Kd^2)/KG + sqrt(delta);
    arg = arg / 2;
    arg = max(-1, min(1, arg));   % clamp [-1, +1]

    theta = acos(arg);            % solution principale
    sin_theta = sin(theta);

    if abs(sin_theta) < 1e-12
        val = 1e6;
        return
    end

    % equation T_rc = Kd * theta / (KG * sin(theta))
    val = Kd * theta / (KG * sin_theta) - Trc;
end