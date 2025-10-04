# GitHub Repository Setup Guide

This guide provides step-by-step instructions to create the GitHub repository and synchronize your local System Monitor project.

## Step 1: Create GitHub Repository

### Option A: Via GitHub Web Interface

1. **Log in to GitHub**: Go to https://github.com and sign in

2. **Create New Repository**:
   - Click the `+` icon in the top right corner
   - Select "New repository"
   
3. **Repository Settings**:
   ```
   Repository name: system-monitor
   Description: Cross-platform system resource monitoring application with FTXUI - Security-first, privacy-by-default terminal UI
   
   Visibility: [Public] or [Private] - your choice
   
   ⚠️ IMPORTANT: DO NOT check any of these:
   [ ] Add a README file
   [ ] Add .gitignore
   [ ] Choose a license
   
   (We already have these files locally)
   ```

4. **Click "Create repository"**

### Option B: Via GitHub CLI (gh)

```bash
# Install GitHub CLI first if needed:
# Windows: choco install gh
# Linux: sudo apt install gh
# macOS: brew install gh

# Authenticate
gh auth login

# Create repository
cd C:\SystemMonitor
gh repo create system-monitor --public --source=. --remote=origin --description="Cross-platform system resource monitoring with FTXUI - Security-first, privacy-by-default"

# Skip to Step 3 (push)
```

## Step 2: Connect Local Repository to GitHub

After creating the repository on GitHub, you'll see a page with setup instructions. Use these commands:

```powershell
# Navigate to your project (if not already there)
cd C:\SystemMonitor

# Add GitHub as remote origin
git remote add origin https://github.com/YOUR_USERNAME/system-monitor.git

# Verify remote was added
git remote -v

# Rename branch to main (if it's master)
git branch -M main
```

**Replace `YOUR_USERNAME` with your actual GitHub username!**

## Step 3: Push to GitHub

```powershell
# Push all commits to GitHub
git push -u origin main
```

You may be prompted for GitHub credentials. If using HTTPS:
- Username: your GitHub username
- Password: use a Personal Access Token (not your password)
  - Create token at: https://github.com/settings/tokens
  - Required scopes: `repo` (full control of private repositories)

## Step 4: Verify Upload

1. Go to https://github.com/YOUR_USERNAME/system-monitor
2. You should see:
   - All 38 files
   - README.md displayed on the main page
   - 3 commits in history
   - All folders (src, include, docs, examples, .github)

## Step 5: Configure Repository Settings

### Enable GitHub Actions

1. Go to repository → Settings → Actions → General
2. Under "Actions permissions", select:
   - ✅ Allow all actions and reusable workflows
3. Click "Save"

### Enable Security Features

1. Go to Settings → Security → Code security and analysis

2. Enable:
   - ✅ Dependency graph
   - ✅ Dependabot alerts
   - ✅ Dependabot security updates
   - ✅ Secret scanning (for public repos)
   - ✅ Code scanning (CodeQL analysis)

### Add Repository Topics

1. Go to main repository page
2. Click ⚙️ (gear icon) next to "About"
3. Add topics:
   ```
   cpp, cpp20, ftxui, terminal-ui, system-monitor, cross-platform,
   windows, linux, macos, security, privacy, monitoring, tui,
   real-time, performance
   ```
4. Click "Save changes"

### Set Repository Description

In the same "About" section:
```
Description: Cross-platform system resource monitoring application with FTXUI. 
Security-first, privacy-by-default terminal UI for real-time CPU, memory, 
disk, and network monitoring. Built with modern C++20.

Website: [Leave blank or add if you create GitHub Pages]
```

## Step 6: Create GitHub Actions Workflow

The workflows are already in `.github/workflows/`, but you need to trigger them:

```powershell
# Make a small change to trigger CI
cd C:\SystemMonitor
git commit --allow-empty -m "Trigger CI/CD pipeline"
git push
```

Go to repository → Actions tab to see the builds running.

## Step 7: Create Release (Optional)

### Via GitHub Web Interface

1. Go to repository → Releases
2. Click "Create a new release"
3. Fill in:
   ```
   Tag version: v1.0.0
   Release title: System Monitor v1.0.0 - Initial Release
   
   Description:
   ## 🎉 First Release
   
   Cross-platform system resource monitoring application with modern C++20 and FTXUI.
   
   ### ✨ Features
   - Real-time CPU, memory, disk, and network monitoring
   - Interactive terminal UI with process tree
   - Cross-platform: Windows, Linux, macOS
   - Security-first design with OWASP compliance
   - Privacy-by-default: no data collection
   - <1% CPU overhead, ~20MB memory footprint
   
   ### 📦 Installation
   See [README.md](README.md) and [DEMO_INSTRUCTIONS.md](docs/DEMO_INSTRUCTIONS.md)
   
   ### 🔒 Security
   - GitLeaks scanning integrated
   - SBOM tracking
   - No hardcoded secrets
   - Memory-safe implementation
   
   ### 📚 Documentation
   - [Architecture](docs/ARCHITECTURE.md)
   - [Security Policy](SECURITY.md)
   - [Contributing](CONTRIBUTING.md)
   ```

4. Check "Set as the latest release"
5. Click "Publish release"

### Via GitHub CLI

```bash
gh release create v1.0.0 \
  --title "System Monitor v1.0.0 - Initial Release" \
  --notes "First release - see README.md for details"
```

## Step 8: Enable GitHub Pages (Optional Documentation Site)

1. Go to Settings → Pages
2. Source: Deploy from a branch
3. Branch: main
4. Folder: /docs
5. Click "Save"

Your documentation will be available at:
`https://YOUR_USERNAME.github.io/system-monitor/`

## Step 9: Add Repository Badges to README

Add these badges to the top of README.md:

```markdown
# System Monitor

[![CI/CD](https://github.com/YOUR_USERNAME/system-monitor/workflows/CI%2FCD/badge.svg)](https://github.com/YOUR_USERNAME/system-monitor/actions)
[![Security](https://github.com/YOUR_USERNAME/system-monitor/workflows/Security/badge.svg)](https://github.com/YOUR_USERNAME/system-monitor/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)](https://github.com/YOUR_USERNAME/system-monitor)
```

Commit and push:
```powershell
git add README.md
git commit -m "Add repository badges"
git push
```

## Step 10: Set Up Branch Protection (Recommended)

1. Go to Settings → Branches
2. Click "Add branch protection rule"
3. Branch name pattern: `main`
4. Enable:
   - ✅ Require a pull request before merging
   - ✅ Require status checks to pass before merging
   - ✅ Require conversation resolution before merging
   - ✅ Include administrators (optional but recommended)
5. Click "Create"

## Verification Checklist

After completing all steps, verify:

- [ ] Repository is accessible at https://github.com/YOUR_USERNAME/system-monitor
- [ ] All 38 files are present
- [ ] README displays correctly on main page
- [ ] GitHub Actions workflows are enabled and running
- [ ] Security features are enabled
- [ ] Repository topics are added
- [ ] Description is set
- [ ] (Optional) Release v1.0.0 is created
- [ ] (Optional) GitHub Pages is enabled
- [ ] (Optional) Badges are displayed in README

## Common Issues

### Authentication Failed

**Problem**: Push fails with authentication error

**Solution**:
```powershell
# Use Personal Access Token instead of password
# Create at: https://github.com/settings/tokens
# Or set up SSH key: https://docs.github.com/en/authentication/connecting-to-github-with-ssh
```

### Actions Not Running

**Problem**: GitHub Actions workflows not executing

**Solution**:
1. Check Settings → Actions → General
2. Ensure "Allow all actions" is selected
3. Make a commit to trigger workflows

### Large File Warning

**Problem**: Warning about files over 50MB

**Solution**: The build artifacts shouldn't be committed (already in .gitignore).
If you see this, ensure build/ directory is gitignored.

## Next Steps After GitHub Sync

1. **Star the repository** (if public) to bookmark it
2. **Watch the repository** to get notifications
3. **Share the link** with colleagues or on social media
4. **Monitor Actions** tab for build status
5. **Review Security** tab for any advisories

## Quick Command Reference

```powershell
# Initial setup
git remote add origin https://github.com/YOUR_USERNAME/system-monitor.git
git branch -M main
git push -u origin main

# Subsequent pushes
git add .
git commit -m "Your commit message"
git push

# Check status
git status
git remote -v
git log --oneline

# Pull latest changes
git pull origin main
```

## Support

If you encounter issues:
1. Check GitHub documentation: https://docs.github.com
2. GitHub Community Forum: https://github.community
3. Contact GitHub Support: https://support.github.com

---

**Congratulations!** 🎉 Your System Monitor project is now on GitHub!

**Repository URL**: https://github.com/YOUR_USERNAME/system-monitor

Remember to replace `YOUR_USERNAME` with your actual GitHub username in all links and commands!

---

Last Updated: October 3, 2025
