/**
 * SkillSwap Application Controller
 * Handles UI interactions, state management, modals, and page rendering.
 */

const App = {
  currentUser: null,
  allStudents: [],

  async init() {
    // Load current user from session or default to Aarav Sharma (ID: 1)
    const storedUser = localStorage.getItem('skillswap_user');
    if (storedUser) {
      this.currentUser = JSON.parse(storedUser);
    } else {
      // Default demo user
      this.currentUser = {
        id: 1,
        name: 'Aarav Sharma',
        email: 'aarav@college.edu',
        dept: 'Computer Science',
        year: 3,
        rating: 4.95,
        rating_count: 12,
        completed_sessions: 14,
        bio: 'FODS Enthusiast and Low-level systems lover. Built custom memory allocators in C.',
        avatar: 'https://images.unsplash.com/photo-1539571696357-5a69c17a67c6?auto=format&fit=crop&w=150&q=80',
        skills_offered: [
          { name: 'C Programming', category: 'Programming', proficiency: 'Expert', format: '1-on-1' },
          { name: 'Data Structures and Algorithms', category: 'Programming', proficiency: 'Advanced', format: '1-on-1' }
        ],
        skills_wanted: ['React', 'UI/UX Design', 'Machine Learning']
      };
      localStorage.setItem('skillswap_user', JSON.stringify(this.currentUser));
    }

    // Load all students for switcher
    try {
      this.allStudents = await SkillSwapAPI.getStudents();
    } catch (e) {}

    this.renderHeaderUser();
    this.setupAutocomplete();
    this.setupUndoButton();
  },

  renderHeaderUser() {
    const nameEl = document.getElementById('header-user-name');
    const deptEl = document.getElementById('header-user-dept');
    const avatarEl = document.getElementById('header-user-avatar');

    if (nameEl && this.currentUser) nameEl.textContent = this.currentUser.name;
    if (deptEl && this.currentUser) deptEl.textContent = `${this.currentUser.dept} • Year ${this.currentUser.year}`;
    if (avatarEl && this.currentUser && this.currentUser.avatar) {
      avatarEl.src = this.currentUser.avatar;
    }
  },

  switchUser(studentId) {
    const student = this.allStudents.find(s => s.id === parseInt(studentId));
    if (student) {
      this.currentUser = student;
      localStorage.setItem('skillswap_user', JSON.stringify(student));
      this.showToast(`Switched active profile to ${student.name}`, 'info');
      setTimeout(() => window.location.reload(), 400);
    }
  },

  setupAutocomplete() {
    const searchInput = document.getElementById('global-search-input');
    const dropdown = document.getElementById('autocomplete-dropdown');
    if (!searchInput || !dropdown) return;

    let debounceTimer;
    searchInput.addEventListener('input', (e) => {
      clearTimeout(debounceTimer);
      const val = e.target.value.trim();
      if (val.length < 1) {
        dropdown.style.display = 'none';
        return;
      }

      debounceTimer = setTimeout(async () => {
        // Calls C Trie prefix autocomplete
        const data = await SkillSwapAPI.searchSkills(val);
        if (data && data.results && data.results.length > 0) {
          dropdown.innerHTML = data.results.map(skill => `
            <div class="autocomplete-item" onclick="App.selectSearchSkill('${skill}')">
              <span><i class="lucide-sparkles" style="color:var(--primary); margin-right:8px;"></i>${skill}</span>
              <span class="badge badge-primary" style="font-size:0.65rem;">Trie Match</span>
            </div>
          `).join('');
          dropdown.style.display = 'block';
        } else {
          dropdown.innerHTML = '<div style="padding:10px 14px; color:var(--text-muted); font-size:0.85rem;">No skills matching prefix</div>';
          dropdown.style.display = 'block';
        }
      }, 150);
    });

    document.addEventListener('click', (e) => {
      if (!searchInput.contains(e.target) && !dropdown.contains(e.target)) {
        dropdown.style.display = 'none';
      }
    });
  },

  selectSearchSkill(skillName) {
    const dropdown = document.getElementById('autocomplete-dropdown');
    if (dropdown) dropdown.style.display = 'none';
    window.location.href = `marketplace.html?search=${encodeURIComponent(skillName)}`;
  },

  setupUndoButton() {
    const undoBtn = document.getElementById('global-undo-btn');
    if (!undoBtn) return;

    undoBtn.addEventListener('click', async () => {
      try {
        const res = await SkillSwapAPI.undoAction();
        this.showToast(`Reverted action: ${res.action_undone.details} (Stack popped)`, 'success');
        setTimeout(() => window.location.reload(), 800);
      } catch (err) {
        this.showToast(err.message || 'No recent actions in undo stack', 'warning');
      }
    });
  },

  showToast(message, type = 'info') {
    let container = document.querySelector('.toast-container');
    if (!container) {
      container = document.createElement('div');
      container.className = 'toast-container';
      document.body.appendChild(container);
    }

    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    let icon = 'info';
    if (type === 'success') icon = 'check-circle';
    if (type === 'error') icon = 'alert-triangle';
    if (type === 'warning') icon = 'alert-circle';

    toast.innerHTML = `
      <i class="lucide-${icon}" style="font-size:1.2rem;"></i>
      <span style="flex:1;">${message}</span>
      <button onclick="this.parentElement.remove()" style="background:none; border:none; color:var(--text-muted); cursor:pointer;">&times;</button>
    `;

    container.appendChild(toast);
    setTimeout(() => {
      toast.style.opacity = '0';
      toast.style.transform = 'translateX(50px)';
      toast.style.transition = 'all 0.3s ease';
      setTimeout(() => toast.remove(), 300);
    }, 4000);
  },

  openModal(modalId) {
    const el = document.getElementById(modalId);
    if (el) el.style.display = 'flex';
  },

  closeModal(modalId) {
    const el = document.getElementById(modalId);
    if (el) el.style.display = 'none';
  }
};

document.addEventListener('DOMContentLoaded', () => {
  App.init();
});
