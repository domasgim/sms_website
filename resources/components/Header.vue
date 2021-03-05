<template>
  <nav>
    <router-link to="/" exact>
      <img class="img" src="/images/Logo.png" alt="Logo" />
    </router-link>
    <div class="relative">
      <router-link class="nav-link" to="/" exact>
        Received messages
      </router-link>
      <span v-if="receivedMessages !== 0" :class="{'over-10': receivedMessages > 9, 'over-100': receivedMessages === '99+'}">{{ receivedMessages }}</span>
    </div>
    <router-link class="nav-link" to="/sent" exact>
      Sent messages
    </router-link>
    <router-link class="nav-link" to="/create" exact>
      Send message
    </router-link>
  </nav>
</template>
<script>
import Button from "./Button";
export default {
  name: "Header",
  components: { Button },
  computed: {
    receivedMessages () {
      const count = this.$store.getters.getUnreadMessages.length
      return count >= 100 ? '99+' : count
    }
  }
};
</script>
<style lang="scss" scoped>
$blue: #0054a6;
$gray: #828282;
$orange: #e9983c;
nav {
  display: block;
  width: 100%;
  display: flex;
  padding-bottom: 1em;
  margin-bottom: 2em;
  border-bottom: 5px solid $blue;
  :nth-child(2) {
    margin-left: auto;
  }
}
.nav-link {
  font-weight: 700;
  font-size: 1em;
  line-height: 50px;
  height: 42px;
  padding-left: 1em;
  text-transform: uppercase;
  color: $blue;
  text-align: center;
  text-decoration: none;
  opacity: 0.5;
  &:hover {
    opacity: 1;
  }
}

.router-link-active {
  opacity: 1;
}

.relative {
  position: relative;
  span {
    position: absolute;
    top: -1px;
    right: -15px;
    padding: 3px 8px;
    color: #fff;
    background-color: #ff0000;
    border-radius: 50%;
    font-size: 0.8em;
    font-weight: 700;
  }
  .over-10 {
    padding: 5px 6px;
    top: -5px;
  }
  .over-100 {
    padding: 5px 3px;
    top: -5px;
  }
}
@media only screen and (max-width: 780px) {
  nav {
    flex-direction: column;
    & > :nth-child(n+0) {
      margin: 0 auto;
    }
    .relative {
      height: 42px;
    }
  }
}
</style>
