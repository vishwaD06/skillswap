/**
 * SkillSwap API Client
 * Seamlessly interfaces with the native C backend HTTP server (port 8080).
 * Provides live connection status and fallbacks.
 */

const API_BASE = window.location.port === '8080' ? '/api' : 'http://localhost:8080/api';

const SkillSwapAPI = {
  isServerConnected: false,
  serverMetadata: null,

  async checkServerStatus() {
    try {
      const res = await fetch(`${API_BASE}/status`, { method: 'GET', signal: AbortSignal.timeout(1800) });
      if (res.ok) {
        this.serverMetadata = await res.json();
        this.isServerConnected = true;
        this.updateConnectionBadge(true);
        return this.serverMetadata;
      }
    } catch (err) {
      this.isServerConnected = false;
      this.updateConnectionBadge(false);
    }
    return null;
  },

  updateConnectionBadge(connected) {
    const badges = document.querySelectorAll('.server-badge');
    badges.forEach(b => {
      if (connected) {
        b.innerHTML = '<span class="status-dot"></span> C Engine: Connected';
        b.style.backgroundColor = 'rgba(16, 185, 129, 0.12)';
        b.style.borderColor = 'rgba(16, 185, 129, 0.3)';
        b.style.color = '#34d399';
        b.title = 'Real-time MinGW GCC C Backend active on port 8080';
      } else {
        b.innerHTML = '<span class="status-dot" style="background-color:#f59e0b; box-shadow:0 0 8px #f59e0b;"></span> Standalone Engine';
        b.style.backgroundColor = 'rgba(245, 158, 11, 0.12)';
        b.style.borderColor = 'rgba(245, 158, 11, 0.3)';
        b.style.color = '#fbbf24';
        b.title = 'Start skillswap_server.exe to enable native C socket backend';
      }
    });
  },

  async getStudents() {
    try {
      const res = await fetch(`${API_BASE}/students`);
      if (res.ok) return await res.json();
    } catch (e) {}
    return JSON.parse(localStorage.getItem('skillswap_students') || '[]');
  },

  async getStudentById(id) {
    try {
      const res = await fetch(`${API_BASE}/students?id=${id}`);
      if (res.ok) return await res.json();
    } catch (e) {}
    const students = await this.getStudents();
    return students.find(s => s.id === parseInt(id)) || null;
  },

  async login(email, password) {
    try {
      const res = await fetch(`${API_BASE}/auth/login`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ email, password })
      });
      if (res.ok) return await res.json();
      const err = await res.json();
      throw new Error(err.error || 'Login failed');
    } catch (e) {
      if (e.message !== 'Failed to fetch') throw e;
      // Fallback local check
      const students = await this.getStudents();
      const user = students.find(s => s.email.toLowerCase() === email.toLowerCase());
      if (user) return { success: true, user };
      throw new Error('Invalid email or password');
    }
  },

  async register(data) {
    try {
      const res = await fetch(`${API_BASE}/auth/register`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data)
      });
      if (res.ok) return await res.json();
      const err = await res.json();
      throw new Error(err.error || 'Registration failed');
    } catch (e) {
      if (e.message !== 'Failed to fetch') throw e;
      const students = await this.getStudents();
      const newId = students.length + 1;
      const newStudent = { id: newId, rating: 5.0, rating_count: 0, completed_sessions: 0, skills_offered: [], skills_wanted: [], ...data };
      students.push(newStudent);
      localStorage.setItem('skillswap_students', JSON.stringify(students));
      return newStudent;
    }
  },

  async getSkills() {
    try {
      const res = await fetch(`${API_BASE}/skills`);
      if (res.ok) return await res.json();
    } catch (e) {}
    return JSON.parse(localStorage.getItem('skillswap_skills') || '[]');
  },

  async addSkill(skillData) {
    try {
      const res = await fetch(`${API_BASE}/skills`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(skillData)
      });
      if (res.ok) return await res.json();
    } catch (e) {}
    const skills = await this.getSkills();
    const newSkill = { id: skills.length + 1, rating: 5.0, completed_sessions: 0, popularity: 50, ...skillData };
    skills.push(newSkill);
    localStorage.setItem('skillswap_skills', JSON.stringify(skills));
    return newSkill;
  },

  // Calls C Trie Autocomplete engine
  async searchSkills(prefix) {
    if (!prefix || prefix.trim().length === 0) return { results: [] };
    try {
      const res = await fetch(`${API_BASE}/search?q=${encodeURIComponent(prefix.trim())}`);
      if (res.ok) return await res.json();
    } catch (e) {}
    // Fallback simulation of Trie prefix matching
    const skills = await this.getSkills();
    const p = prefix.toLowerCase();
    const results = [...new Set(skills.filter(s => s.name.toLowerCase().startsWith(p)).map(s => s.name))];
    return { prefix, algorithm: 'Trie (Prefix Tree)', count: results.length, results };
  },

  // Calls C Graph Matching engine
  async getRecommendations(studentId) {
    try {
      const res = await fetch(`${API_BASE}/recommendations?student_id=${studentId}`);
      if (res.ok) return await res.json();
    } catch (e) {}
    return { recommendations: [] };
  },

  // Calls C Max-Heap Priority Queue Leaderboard engine
  async getLeaderboard() {
    try {
      const res = await fetch(`${API_BASE}/leaderboard`);
      if (res.ok) return await res.json();
    } catch (e) {}
    return { leaderboard: [] };
  },

  async getRequests(studentId) {
    try {
      const url = studentId ? `${API_BASE}/requests?student_id=${studentId}` : `${API_BASE}/requests`;
      const res = await fetch(url);
      if (res.ok) return await res.json();
    } catch (e) {}
    return JSON.parse(localStorage.getItem('skillswap_requests') || '[]');
  },

  // Enqueues into C FIFO Queue
  async sendRequest(requestData) {
    try {
      const res = await fetch(`${API_BASE}/requests`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(requestData)
      });
      if (res.ok) return await res.json();
      const err = await res.json();
      throw new Error(err.error || 'Failed to send request');
    } catch (e) {
      if (e.message !== 'Failed to fetch') throw e;
      const reqs = await this.getRequests();
      const newReq = { id: reqs.length + 1, status: 'PENDING', created_at: new Date().toISOString().slice(0, 16).replace('T', ' '), ...requestData };
      reqs.push(newReq);
      localStorage.setItem('skillswap_requests', JSON.stringify(reqs));
      return newReq;
    }
  },

  async handleRequestAction(requestId, action) {
    try {
      const res = await fetch(`${API_BASE}/requests/action`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ request_id: requestId, action })
      });
      if (res.ok) return await res.json();
    } catch (e) {}
    const reqs = await this.getRequests();
    const req = reqs.find(r => r.id === requestId);
    if (req) {
      req.status = action;
      localStorage.setItem('skillswap_requests', JSON.stringify(reqs));
    }
    return req;
  },

  // Pops C LIFO Stack for Undo
  async undoAction() {
    try {
      const res = await fetch(`${API_BASE}/undo`, { method: 'POST' });
      if (res.ok) return await res.json();
      const err = await res.json();
      throw new Error(err.error || 'Nothing to undo');
    } catch (e) {
      throw new Error(e.message || 'Undo operation failed');
    }
  },

  async getSessions(studentId) {
    try {
      const url = studentId ? `${API_BASE}/sessions?student_id=${studentId}` : `${API_BASE}/sessions`;
      const res = await fetch(url);
      if (res.ok) return await res.json();
    } catch (e) {}
    return JSON.parse(localStorage.getItem('skillswap_sessions') || '[]');
  },

  async submitReview(sessionId, rating, feedback) {
    try {
      const res = await fetch(`${API_BASE}/sessions/review`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ session_id: sessionId, rating, feedback })
      });
      if (res.ok) return await res.json();
      const err = await res.json();
      throw new Error(err.error || 'Failed to submit review');
    } catch (e) {
      if (e.message !== 'Failed to fetch') throw e;
      const sess = await this.getSessions();
      const s = sess.find(item => item.id === sessionId);
      if (s) {
        s.has_review = true;
        s.rating = rating;
        s.feedback = feedback;
        s.status = 'COMPLETED';
        localStorage.setItem('skillswap_sessions', JSON.stringify(sess));
      }
      return s;
    }
  },

  async getGraphData() {
    try {
      const res = await fetch(`${API_BASE}/graph`);
      if (res.ok) return await res.json();
    } catch (e) {}
    return { nodes: [], edges: [] };
  },

  async runDSDemo(type) {
    try {
      const res = await fetch(`${API_BASE}/ds-demo`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ type })
      });
      if (res.ok) return await res.json();
    } catch (e) {}
    return null;
  }
};

// Check server status immediately on load
document.addEventListener('DOMContentLoaded', () => {
  SkillSwapAPI.checkServerStatus();
  setInterval(() => SkillSwapAPI.checkServerStatus(), 8000);
});
