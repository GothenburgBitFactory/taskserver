# We use docker's multistage build process 
# to reduce final image size.
#
# If we use the same image we build with,
# the image would be over 400MB(!).
#
# Current size is around 95MB, 
# but improvements can be made by compiling 
# statically and putting it in an alpine image.

# we use a debian image
FROM debian:8-slim as buildimage
WORKDIR /build

# we add the dependencies specified by the guide
RUN apt-get update \
  && apt-get install -y \
    build-essential \
    g++ \
    libgnutls28-dev \
    uuid-dev \
    cmake \
    gnutls-bin \
  && rm -rf /var/cache/apt/

COPY ./CMakeLists.txt /build/
COPY ./src /build/src
COPY ./doc /build/doc
COPY ./cmake.* /build/
COPY ./cmake /build/cmake

RUN cmake -DCMAKE_BUILD_TYPE=release . 
RUN make -j3

WORKDIR /pki
COPY pki/ /pki
RUN ./generate



FROM debian:8-slim as finalimage
EXPOSE 53589

RUN apt-get update \
  && apt-get install -y \
    libgnutlsxx28 \
    uuid \
  && rm -rf /var/cache/apt/


COPY --from=buildimage /build/src/taskd /
RUN chmod +x /taskd

ENTRYPOINT ["/taskd"]
CMD ["server"]

ENV TASKDDATA=/data
RUN mkdir -p /data

COPY example-config /data/config

COPY --from=buildimage pki/*.pem /certificates/

VOLUME /data
VOLUME /certificates
