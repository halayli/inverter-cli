#!/bin/bash

CREATE_STMT=$(
  cat <<'EOF'
CREATE TABLE IF NOT EXISTS inverter_metrics (
    id SERIAL PRIMARY KEY,
    created TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    data json
);
EOF
)

nc -c -u -l 12000 |
  while IFS= read -r EVENT; do
    psql iv -c "INSERT INTO inverter_metrics (data) values('$EVENT');"
  done
