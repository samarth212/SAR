# ---- build stage ----
FROM ubuntu:24.04 AS build

ARG UBUNTU_CODENAME=noble

RUN apt-get update && apt-get install -y --no-install-recommends \
  ca-certificates curl gnupg \
  && mkdir -p /etc/apt/keyrings \
  && curl -fsSL https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /etc/apt/keyrings/kitware-archive-keyring.gpg \
  && echo "deb [signed-by=/etc/apt/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ ${UBUNTU_CODENAME} main" > /etc/apt/sources.list.d/kitware.list \
  && apt-get update && apt-get install -y --no-install-recommends \
  build-essential cmake ninja-build git zip unzip pkg-config \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build vcpkg (from submodule) and build the service
RUN ./external/vcpkg/bootstrap-vcpkg.sh
RUN cmake --preset dev
RUN cmake --build --preset dev

# ---- runtime stage ----
FROM ubuntu:24.04 AS runtime

WORKDIR /app
COPY --from=build /app/build/dev/services/hello-service/hello-service /app/hello-service

ENV PORT=7001
EXPOSE 7001

CMD ["/app/hello-service"]
