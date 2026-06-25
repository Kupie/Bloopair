# Reproducible Bloopair build environment for local development, agents, and CI.
# Keep this image in sync with .github/workflows/ci.yml.
FROM devkitpro/devkitppc:20260221

ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITPPC=/opt/devkitpro/devkitPPC
ENV DEVKITARM=/opt/devkitpro/devkitARM
ENV PATH=/opt/devkitpro/tools/bin:/opt/devkitpro/devkitPPC/bin:/opt/devkitpro/devkitARM/bin:${PATH}

USER root
RUN dkp-pacman -Sy --noconfirm --needed \
	devkitARM \
	devkitPPC \
	wiiu-dev \
	general-tools \
	&& dkp-pacman -Scc --noconfirm

WORKDIR /workspace/Bloopair
CMD ["make", "dist"]
